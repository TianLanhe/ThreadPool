#ifndef DATA_STRUCTURE_BASE_H
#define DATA_STRUCTURE_BASE_H

#define OK 0
#define ER 1
#define INFEASIBLE -1

#ifndef ElemType
#define ElemType int
#endif

typedef int Status;

#define BEFORE_CHECK_RESULT()	\
	Status __status

#define CHECK_RESULT(func)		\
	if((__status = func)!=OK){	\
		return __status;		\
	}

#define CHECK_RESULT_BY_MSG(func,msg_s,msg_f)	\
	if((__status = func)!=OK){			\
		std::cout << (msg_f) << std::endl;		\
		return __status;				\
	}else{								\
		std::cout << (msg_s) << std::endl;		\
	}

#define AFTER_CHECK_RESULT()	\
	return OK

#define CHECK_OVERFLOW(var)		\
	if(!var){					\
		return OVERFLOW;		\
	}

#define CHECK_ERROR(exp)		\
	if(!(exp))					\
		return ER

#define CHECK_ERROR_BY_MSG(exp,msg_s,msg_f)	\
	if(!(exp)){						\
		std::cout << (msg_f) << std::endl;	\
		return ER;					\
	}else{							\
		std::cout << (msg_s) << std::endl;	\
	}

#define CHECK_ERROR_BY_MSG2(exp,msg_f)	\
	if(!(exp)){							\
		std::cout << (msg_f) << std::endl;	\
		return ER;							\
	}

#endif
