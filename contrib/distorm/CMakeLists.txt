
add_library(distorm
                decoder.c
                distorm.c
                instructions.c
                insts.c
                mnemonics.c
                operands.c
                prefix.c
                textdefs.c
                wstring.c)

#target_link_libraries(mongoose pthread wsock32)
