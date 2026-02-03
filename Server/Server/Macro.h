#ifdef _DEBUG
	#define assert(expression) ((void)(                                                       \
			(!!(expression)) ||                                                               \
			(my_assert_function(#expression, __FILE__, __LINE__), 0))                         \
		)
#else
#define assert(expression) ((void)0)
#endif