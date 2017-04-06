package main

import (
	"C"
	"errors"
	"fmt"
	"unsafe"
)

//export processRequestDone
func processRequestDone(ctx unsafe.Pointer, str *C.char) {
	output_ctx := (*CtxOutput)(ctx)
	output_ctx.response = C.GoString(str)
}

//export logCallback
func logCallback(str *C.char) {
	fmt.Println(C.GoString(str))
}

//export logCallbackWithCtx
func logCallbackWithCtx(ctx unsafe.Pointer, str *C.char) {
	output_ctx := (*CtxOutput)(ctx)
	fmt.Println(output_ctx.id, C.GoString(str))
}

//export processRequestError
func processRequestError(ctx unsafe.Pointer, str *C.char) {
	output_ctx := (*CtxOutput)(ctx)
	fmt.Println(output_ctx.id, "Got error during request processing")
	output_ctx.err = errors.New("Failed to find solution")
	output_ctx.response = C.GoString(str)
}
