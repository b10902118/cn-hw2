#pragma once

#define ERR_EXIT(a)                                                         \
    {                                                                       \
        perror(a);                                                          \
        exit(1);                                                            \
    }
