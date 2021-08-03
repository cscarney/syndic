set(htmlparser_HEADERS
    attribute.h
    char_ref.h
    error.h gumbo.h
    gumbo_edit.h
    insertion_mode.h
    parser.h
    replacement.h
    string_buffer.h
    string_piece.h
    tag_enum.h
    tag_perf.h
    tag_sizes.h
    tag_strings.h
    token_type.h
    tokenizer.h
    tokenizer_states.h
    utf8.h
    util.h
    vector.h
    )

set(htmlparser_SRCS
    ${htmlparser_HEADERS}
    attribute.c 
    char_ref.c 
    error.c 
    gumbo_edit.c 
    parser.c 
    string_buffer.c 
    string_piece.c 
    svg_attrs.c 
    svg_tags.c 
    tag.c 
    tokenizer.c 
    utf8.c 
    util.c 
    vector.c)
    
add_library(htmlparser STATIC ${htmlparser_SRCS})
target_link_libraries(htmlparser)
