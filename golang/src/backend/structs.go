package main

import (
	"github.com/satori/go.uuid"
)

type RequestSettings struct {
	MeanTruckSpeed       int32  `json:"mean_track_speed_km_in_hour"`
	ColonySizeLimit      int32  `json:"colony_size_limit"`
	TrialCountMaxLimit   int32  `json:"trial_count_max_limit"`
	MaxTimeInSec         int64  `json:"max_time_in_sec"`
	MaxInactiveTimeInSec int64  `json:"max_inactive_time_in_sec"`
	LogLevel             string `json:"log_level"`
	PartialResultEnabled bool   `json:"partial_solution_enabled"`
	MultiThreaded        bool   `json:"multi_threaded"`
}
type LibrarySettings struct {
	OsrmPath *string `json:"osrm_path"`
}

type JsonRequest struct {
	Account_id int32           `json:"account_id"`
	Vehicles   []interface{}   `json:"vehicles"`
	Zones      []int32         `json:"zones_"`
	Zone_descs []interface{}   `json:"zones"`
	Tasks      []interface{}   `json:"tasks"`
	Settings   RequestSettings `json:"settings"`
}

type OutputError struct {
	Code  uint32 `json:"code"`
	Error string `json:"error"`
}

type CtxOutput struct {
	err      error
	response string
	id       uuid.UUID
}

type TaskResponse struct {
	TimeStart uint64 `json:"ts"`
	TimeEnd   uint64 `json:"te"`
	Action    string `json:"action"`
	Id        uint32 `json:"id"`
}
type TripTesponse struct {
	TimeStart uint64         `json:"ts"`
	TimeEnd   uint64         `json:"te"`
	Vehicle   uint32         `json:"vehicle"`
	Length    float64        `json:"length"`
	Tasks     []TaskResponse `json:"tasks"`
}

type JsonResponse struct {
	AccountId        int32          `json:"account_id"`
	CalculationTime  uint32         `json:"calculation_time"`
	CalculationStart uint64         `json:"calculation_start"`
	Trips            []TripTesponse `json:"trips"`
	Errors           []interface{}  `json:"errors"`
}
