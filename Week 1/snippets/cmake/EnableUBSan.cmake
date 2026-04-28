# UndefinedBehaviorSanitizer — optional.

add_compile_options(-fsanitize=undefined -g -fno-omit-frame-pointer)
add_link_options(-fsanitize=undefined)

message(STATUS "UBSan enabled")
