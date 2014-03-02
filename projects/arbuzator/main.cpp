#include <iostream>
#include <fstream>

#include <pe_bliss/pe_bliss.h>
#include <distorm/distorm.h>

// /home/fippo/Downloads/mingw/bin/i686-w64-mingw32-c++ test.c

using namespace pe_bliss;
using namespace std;

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

    try
    {
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


        imported_functions_list imports = get_imported_functions(image);

//        for (size_t i = 0; i < imports.size(); ++i) {
//            import_library& lib = imports[i];
//            const import_library::imported_list& imported_funcs = lib.get_imported_functions();
//            for (size_t j = 0; j < imported_funcs.size(); ++i) {
//                const imported_function& func = imported_funcs[j];
//            }
//        }

        std::cout << "Sections: " << image.get_number_of_sections() << std::endl;

        section_list& sections = image.get_image_sections();
        section_list newSections;

        for (size_t i = 0; i < sections.size(); ++i) {
            section sec = sections[i];
            cout << sec.get_name() << "\n";
            if (!sec.executable()) {
                newSections.push_back(sec);
                continue;
            }

            std::string data = sec.get_raw_data();
            data.resize(sec.get_virtual_size());

            unsigned int usedInstructionsCount;
            vector<_DecodedInst> instructions;
            instructions.resize(100000);
            _DecodeType dt = Decode32Bits;
            distorm_decode(0, (const unsigned char*)&data[0], data.size(), dt, &instructions[0], instructions.size(), &usedInstructionsCount);
            instructions.resize(usedInstructionsCount);

            string newData;
            for (size_t i = 0; i < instructions.size(); ++i) {
                _DecodedInst& inst = instructions[i];
                newData += data.substr(inst.offset, inst.size);
//                cout << inst.mnemonic.p << " " << inst.operands.p << "\n";
//                newData.push_back(0x90);
            }

            for (size_t i = 0; i < 5000; ++i) {
                newData.push_back(0x90);
            }

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

//        import_rebuilder_settings settings(true, false); //Модифицируем заголовок PE и не очищаем поле IMAGE_DIRECTORY_ENTRY_IAT
//        rebuild_imports(image, imports, attached_section, settings); //Пересобираем импорты
        image_directory dir = rebuild_imports(*new_image, imports, attached_section);

        //Пересобираем PE-файл из нового обраща
        rebuild_pe(*new_image, new_pe_file);

    }
    catch(const pe_exception& e)
    {
        //Если возникла ошибка
        std::cout << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
