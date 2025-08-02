// NOT PART OF LLVM, ONLY MISSING PARTS FOR MINIMAL COMPILATION
#ifndef PROFILE_INSTRPROFILING_H
#define PROFILE_INSTRPROFILING_H

#define COMPILER_RT_VISIBILITY

#define O_BINARY 0

#define PROF_ERR(Format, ...) \
    fprintf(stderr, "LLVM Profile Error: " Format, __VA_ARGS__);

#define PROF_WARN(Format, ...) \
    fprintf(stderr, "LLVM Profile Warning: " Format, __VA_ARGS__);

#define PROF_NOTE(Format, ...) \
    fprintf(stderr, "LLVM Profile Note: " Format, __VA_ARGS__);

#define IS_DIR_SEPARATOR(ch) ((ch) == DIR_SEPARATOR)
#define DIR_SEPARATOR '/'

/*
 * madvise() flags
 */
#define MADV_NORMAL 0   /* no special treatment */
#define MADV_WILLNEED 3 /* expect access in the near future */
#define MADV_DONTNEED 4 /* do not expect access in the near future */

#endif // PROFILE_INSTRPROFILING_H
