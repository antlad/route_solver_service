#ifndef CREATE_ROUTE_CLIB_INTERFACE_HPP
#define CREATE_ROUTE_CLIB_INTERFACE_HPP

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GOLANG
void processRequestDone(void* ctx, const char* str);
void processRequestError(void* ctx, const char* str);
void logCallbackWithCtx(void* ctx, const char* str);
void logCallback(const char* str);
#endif

int setup(const char* settings);
int processRequest(void* ctx, const char* json);

#ifdef __cplusplus
}
#endif

#endif //PROJECT_SIZE2D_INTERFACE_HPP
