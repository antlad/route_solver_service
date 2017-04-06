package main

// #cgo CFLAGS: -DGOLANG=1
// #cgo CPPFLAGS: -I../../../include/wrapper/
// #cgo LDFLAGS: -lwrapper
// #include "interface.hpp"
import "C"

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"

	"flag"
	"net/http"
	"path"
	"runtime"
	"unsafe"

	"github.com/gin-gonic/gin"
	"github.com/satori/go.uuid"
)

var osrm_path = flag.String("osrm_path", "", "Path to osrm file")
var requests_folder = flag.String("dump_folder", "", "Folder where backend will dump all requests and responses")
var listen = flag.String("listen", "0.0.0.0:7779", "Listen address and port")

var sem = make(chan int, runtime.NumCPU())

func main() {
	flag.Parse()
	r := gin.Default()
	r.Use(gin.Logger())
	r.Use(gin.Recovery())
	r.GET("/ping", func(c *gin.Context) {
		sem <- 1
		c.JSON(200, gin.H{
			"message": "pong",
		})
		<-sem
	})

	r.POST("/solve_routes", func(c *gin.Context) {
		var request JsonRequest
		request.Settings.ColonySizeLimit = 100
		request.Settings.MaxTimeInSec = 60 * 5
		request.Settings.MaxInactiveTimeInSec = 30
		request.Settings.TrialCountMaxLimit = 50
		request.Settings.MeanTruckSpeed = 90
		request.Settings.PartialResultEnabled = false
		request.Settings.LogLevel = "debug"
		request.Settings.MultiThreaded = false

		if c.BindJSON(&request) == nil {

			b, err := json.Marshal(request)
			if err != nil {
				c.JSON(http.StatusBadRequest, err.Error())
				return
			}

			output_ctx := &CtxOutput{
				id: uuid.NewV4(),
			}

			if len(*requests_folder) > 0 {
				if err := ioutil.WriteFile(path.Join(*requests_folder, output_ctx.id.String()+"_input.json"), b, 0644); err != nil {
					fmt.Println("Can't write request to dump folder ", err.Error())
				}
			}

			sem <- 1
			rcode := C.processRequest(unsafe.Pointer(output_ctx), (*C.char)(unsafe.Pointer(&b[0])))
			<-sem
			if rcode != 0 {
				out_err := &OutputError{
					Error: output_ctx.response,
					Code:  http.StatusBadRequest,
				}
				c.JSON(http.StatusBadRequest, out_err)
				return
			}

			out_bytes := []byte(output_ctx.response)

			if len(*requests_folder) > 0 {
				if err := ioutil.WriteFile(path.Join(*requests_folder, output_ctx.id.String()+"_output.json"), out_bytes, 0644); err != nil {
					fmt.Println("Can't write response to dump folder ", err.Error())
				}
			}

			c.Writer.Header().Set("Content-Type", "application/json")
			c.String(http.StatusOK, output_ctx.response)
		}
	})

	settings := LibrarySettings{
		OsrmPath: osrm_path,
	}

	b, err := json.Marshal(settings)
	if err != nil {
		fmt.Fprintln(os.Stderr, "Error during json marshal ", err.Error())
		os.Exit(1)
	}

	if C.setup((*C.char)(unsafe.Pointer(&b[0]))) != 0 {
		fmt.Fprintln(os.Stderr, "Error during library set up")
		os.Exit(1)
	}

	r.Run(*listen)
}
