#include <iostream>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <assert.h>

#include <pe_bliss/pe_bliss.h>
#include <distorm/distorm.h>
#include <distorm/mnemonics.h>

#include <utils/types.h>

#include <boost/optional.hpp>

#include "instruction_remapper.h"

// /home/fippo/Downloads/mingw/bin/i686-w64-mingw32-c++ test.c

using namespace pe_bliss;
using namespace std;

size_t GetOpSize(const _Operand& op) {
    switch (op.type) {
    case O_NONE:
    case O_REG:
        return 0;
    case O_DISP:
        return 4;
    default:
        return op.size / 8;
    }
}

//Пример, показывающий, как получить базовую информацию о PE-файле
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Usage: basic_info_viewer.exe PE_FILE" << std::endl;
        return 0;
    }

    //Открываем файл
    std::ifstream pe_file(argv[1], std::ios::in | std::ios::binary);
    if(!pe_file)
    {
        std::cout << "Cannot open " << argv[1] << std::endl;
        return -1;
    }

//    try
//    {

        TDataRemapper<ui32> dataRemapper;
        TImportFunctionsMapper<ui32> importMapper;
        TInstructionRemapper remapper(dataRemapper, importMapper, "rules.json");

        //Создаем экземпляр PE или PE+ класса с помощью фабрики
        pe_base image(pe_factory::create_pe(pe_file));

        std::auto_ptr<pe_base> new_image;

        std::ofstream new_pe_file("result.exe", std::ios::out | std::ios::binary | std::ios::trunc);
        if(!new_pe_file) {
            std::cout << "Cannot create " << "result.exe" << std::endl;
            return 42;
        }

        //Создаем новый пустой образ
        new_image.reset(image.get_pe_type() == pe_type_32
            ? static_cast<pe_base*>(new pe_base(pe_properties_32(), image.get_section_alignment()))
            : static_cast<pe_base*>(new pe_base(pe_properties_64(), image.get_section_alignment())));

        //Копируем важные параметры старого образа в новый
        new_image->set_characteristics(image.get_characteristics());
        new_image->set_dll_characteristics(image.get_dll_characteristics());
        new_image->set_file_alignment(image.get_file_alignment());
        new_image->set_heap_size_commit(image.get_heap_size_commit_64());
        new_image->set_heap_size_reserve(image.get_heap_size_reserve_64());
        new_image->set_stack_size_commit(image.get_stack_size_commit_64());
        new_image->set_stack_size_reserve(image.get_stack_size_reserve_64());
        new_image->set_image_base_64(image.get_image_base_64());
        new_image->set_ep(image.get_ep());
        new_image->set_number_of_rvas_and_sizes(new_image->get_number_of_rvas_and_sizes());
        new_image->set_subsystem(image.get_subsystem());


        unsigned int image_base;
        image.get_image_base(image_base);

        importMapper.Prepare(image);


        //image_directory_entry_bound_import
        unsigned long current_descriptor_pos = image.get_directory_rva(13);
        cout << "!!!image_directory_entry_bound_import " << current_descriptor_pos << "\n";


        imported_functions_list imports = get_imported_functions(image);

        std::cout << "Sections: " << image.get_number_of_sections() << std::endl;

        section_list& sections = image.get_image_sections();
        section_list newSections;

        pair<size_t, size_t> oldImp;
        pair<size_t, size_t> newImp;



        for (size_t i = 0; i < sections.size(); ++i) {
            section sec = sections[i];
            cout << std::hex << image_base + sec.get_virtual_address() << ": " << sec.get_name() << "\n";
            if (!sec.executable()) {
                if (string(sec.get_name()) != ".idata") {
                    newSections.push_back(sec);
                    dataRemapper.AddSection(sec.get_name(), image_base + sec.get_virtual_address(), sec.get_virtual_size());
                } else {
                    oldImp.first = image_base + sec.get_virtual_address();
                    oldImp.second = sec.get_virtual_size();
                }
                continue;
            }

            std::string data = sec.get_raw_data();
            data.resize(sec.get_virtual_size());

            unsigned int usedInstructionsCount;
            vector<_DecodedInst> instructions;
            instructions.resize(500000);
            _DecodeType dt = Decode32Bits;
            distorm_decode(0, (const unsigned char*)&data[0], data.size(), dt, &instructions[0], instructions.size(), &usedInstructionsCount);
            instructions.resize(usedInstructionsCount);


            string newData;
            for (size_t i = 0; i < instructions.size(); ++i) {
                _DecodedInst& inst = instructions[i];
                newData += data.substr(inst.offset, inst.size);

//                string name = (const char*)inst.mnemonic.p;
//                if (name == "CALL") {
//                    cout << std::hex <<  image_base + sec.get_virtual_address() + inst.offset << "(" <<
//                            inst.offset <<
//                            "): " << inst.mnemonic.p << " " << inst.operands.p << "\n";
//                }
//                newData.push_back(0x90);
            }

//            for (size_t i = 0; i < 800; ++i) {
//                newData.push_back(0x90);
//            }

            cout << "instructions:   " << instructions.size() << "\n";
            cout << "orig data size: " << data.size() << "\n";
            cout << "new data size:  " << newData.size() << "\n";

            sec.set_virtual_size(newData.size());
            size_t newSize = (newData.size() / 512 + 1) * 512;
            newData.resize(newSize);
            sec.set_raw_data(newData);
            sec.set_size_of_raw_data(newData.size());

            newSections.push_back(sec);
        }

        sections.clear();
        for (size_t i = 0; i < newSections.size(); ++i) {
            new_image->set_section_virtual_size(new_image->add_section(newSections[i]), newSections[i].get_virtual_size());
        }

        section new_imports;
        new_imports.get_raw_data().resize(1); //Мы не можем добавлять пустые секции, поэтому пусть у нее будет начальный размер данных 1
        new_imports.set_name("new_imp"); //Имя секции
        new_imports.readable(true).writeable(true); //Доступна на чтение и запись
        section& attached_section = new_image->add_section(new_imports); //Добавим секцию и получим ссылку на добавленную секцию с просчитанными размерами

        import_rebuilder_settings settings(true, false); //Модифицируем заголовок PE и не очищаем поле IMAGE_DIRECTORY_ENTRY_IAT
        settings.save_iat_and_original_iat_rvas(false, true);
        rebuild_imports(*new_image, imports, attached_section, settings);


        section_list& sections1 = new_image->get_image_sections();

        for (size_t i = 0; i < sections1.size(); ++i) {
            section sec = sections1[i];
            cout << std::hex << image_base + sec.get_virtual_address() << ": " << sec.get_name() << "\n";
            if (string(sec.get_name()) == "new_imp") {
                newImp.first = image_base + sec.get_virtual_address();
                newImp.second = sec.get_virtual_size();
            } else {
                if (!sec.executable()) {
                    dataRemapper.AddNewSection(sec.get_name(), image_base + sec.get_virtual_address(), sec.get_virtual_size());
                }
            }
        }

        size_t newImageBase;
        new_image->get_image_base(newImageBase);
        size_t oldImportOffset = image_base + image.get_directory_rva(pe_win::image_directory_entry_import);
        size_t newImportOffset = newImageBase + new_image->get_directory_rva(pe_win::image_directory_entry_import);

        cerr << "old import offset:        " << oldImportOffset << "\n";
        cerr << "imp section offset:       " << oldImp.first << "\n";
        cerr << "new import offset:        " << newImportOffset << "\n";

        importMapper.Update(*new_image);

        for (size_t i = 0; i < sections1.size(); ++i) {

            section& sec = sections1[i];
            if (!sec.executable()) {
                continue;
            }

            std::string data = sec.get_raw_data();
            data.resize(sec.get_virtual_size());

            {
                unsigned int usedInstructionsCount;
                vector<_DecodedInst> instructions;
                instructions.resize(500000);
                _DecodeType dt = Decode32Bits;
                distorm_decode(0, (const unsigned char*)&data[0], data.size(), dt, &instructions[0], instructions.size(), &usedInstructionsCount);
                instructions.resize(usedInstructionsCount);
                cout << "inst size: " << instructions.size() << "\n";
            }


            _CodeInfo codesInfo;
            codesInfo.code = (const unsigned char*)data.data();
            codesInfo.codeLen = data.size();
            codesInfo.dt = Decode32Bits;
            codesInfo.codeOffset = 0;
            codesInfo.features = DF_NONE;

            vector<_DInst> instructions;
            instructions.resize(500000);
            unsigned int usedInstructionsCount = 0;
            distorm_decompose(&codesInfo, &instructions[0], instructions.size(), &usedInstructionsCount);
            instructions.resize(usedInstructionsCount);
            string newData;
            newData.reserve(data.size());

            cout << "instructions: " << instructions.size() << "\n";
            for (size_t j = 0; j < instructions.size(); ++j) {
                _DInst& inst = instructions[j];
                if (inst.flags == FLAG_NOT_DECODABLE) {
                    break;
                }

                size_t opsSize = 0;
                for (size_t k = 0; k < 4; ++k) {
                    _Operand& op = inst.ops[k];
                    opsSize += GetOpSize(op);
                }

                size_t prefixSize = inst.size - opsSize;

                vector<size_t> dispOffsets;
                size_t currentOffset = prefixSize;

                for (size_t k = 0; k < 4; ++k) {
                    _Operand& op = inst.ops[k];
                    bool matched = false;
                    if (op.type == O_DISP) {
                        matched = true;
                    } else if (op.type == O_IMM && op.size == 32) {
                        matched = true;
                    }

                    if (!matched) {
                        currentOffset += GetOpSize(op);
                        continue;
                    }
                    dispOffsets.push_back(currentOffset);
                    currentOffset += GetOpSize(op);
                }

                _DecodedInst decodedInst;
                distorm_format(&codesInfo, &inst, &decodedInst);

                string instData = data.substr(decodedInst.offset, decodedInst.size);

                if (dispOffsets.empty()) {
                    newData += instData;
                    continue;
                }

                printf("%s %s %s - ", decodedInst.instructionHex.p, decodedInst.mnemonic.p, decodedInst.operands.p);
                printf("%s", decodedInst.mnemonic.p);

                for (size_t k = 0; k < 4; ++k) {
                    if (inst.ops[k].type != O_NONE) {
                        switch(inst.ops[k].type) {
                            case O_PC: cout << " PC"; break;
                            case O_SMEM: cout << " SMEM"; break;
                            case O_MEM: cout << " MEM"; break;
                            case O_REG: cout << " REG"; break;
                            case O_DISP: cout << " DISP"; break;
                            case O_IMM: cout << " IMM"; break;
                        default:
                            cout << " " << dec << (unsigned int)inst.ops[k].type;
                        }
                        cout << dec << "(" << inst.ops[k].size << ")";
                        cout << dec << "(" << GetOpSize(inst.ops[k]) << ")";
                    }
                }
                cout << "\n";

                for (size_t k = 0; k < dispOffsets.size(); ++k) {
                    cout << dispOffsets[k] << " ";
                }
                cout << "\n";

                remapper.Remap(instData, dispOffsets);
                newData += instData;

//                cout << dec << (unsigned int)inst.size << " " << opsSize << "\n";
            }


            /*

            unsigned int usedInstructionsCount;
            vector<_DecodedInst> instructions;
            instructions.resize(500000);
            _DecodeType dt = Decode32Bits;
            distorm_decode(0, (const unsigned char*)&data[0], sec.get_virtual_size(), dt, &instructions[0], instructions.size(), &usedInstructionsCount);
            instructions.resize(usedInstructionsCount);

            string newData;

            cout << std::hex;
            for (size_t i = 0; i < instructions.size(); ++i) {
                _DecodedInst& inst = instructions[i];
                string instData = data.substr(inst.offset, inst.size);
                remapper.Remap(instData);
                newData += instData;
            }

            */
            sec.set_raw_data(newData);
        }




        //Пересобираем PE-файл из нового обраща
        rebuild_pe(*new_image, new_pe_file);

//    }
//    catch(const pe_exception& e)
//    {
//        //Если возникла ошибка
//        std::cout << "Error: " << e.what() << std::endl;
//        return -1;
//    }

    return 0;
}
