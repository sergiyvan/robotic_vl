/**
 ** macroMagic.h
 **
 */

#ifndef MACROMAGIC_H_
#define MACROMAGIC_H_

/* Mark unused variables - if needed at all - to suppress
 * gcc -Wunused-but-set-variable warnings */
#define UNUSED(expr) do { (void)(expr); } while (0)

#define CONCATENATE(a, b)    a##b
#define PRECONCATENATE(a, b) CONCATENATE(a, b)
#define UNIQUEVAR            PRECONCATENATE(Unique, __COUNTER__)
#define UNIQUE(lineno)       UNIQUEVAR

#define STRINGIFY2(x)      #x
#define STRINGIFY(x)       STRINGIFY2(x)

#endif /* MACROMAGIC_H_ */
