#ifndef XLOG_H_
#define XLOG_H_

#ifndef _WIN32

#define XCPP_API 

#else

#ifdef xlog_EXPORTS
#define XCPP_API __declspec(dllexport)
#else
#define XCPP_API __declspec(dllimport)
#endif

#endif

class XCPP_API xlog {
    public:
        xlog();
};

#endif
