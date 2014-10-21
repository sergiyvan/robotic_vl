#ifndef SYSTEMBITS_H_
#define SYSTEMBITS_H_

// Based on https://stackoverflow.com/questions/1505582/determining-32-vs-64-bit-in-c

#if defined _WIN32
	#define SYSTEMBITS32
#elif defined _WIN64
	#define SYSTEMBITS64
#elif defined __GNUC__ // gcc
	#if defined __x86_64__ or defined __ppc64__
		#define SYSTEMBITS64
	#else
		#define SYSTEMBITS32
	#endif
#endif


#endif /* SYSTEMBITS_H_ */
