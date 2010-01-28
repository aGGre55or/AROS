#ifndef _FLOATTEXT_PRIVATE_H_
#define _FLOATTEXT_PRIVATE_H_

struct Floattext_DATA
{
    struct Hook  construct_hook;
    struct Hook  destruct_hook;
    struct Hook  display_hook;
    STRPTR       text;
    BOOL         justify;
    STRPTR       skipchars;
    LONG         tabsize;
};

#endif /* _FLOATTEXT_PRIVATE_H_ */
