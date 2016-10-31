#include "sweep.h"
#include "protocol.h"
#include "serial.h"

#include <stdlib.h>

int32_t sweep_get_version(void) { return SWEEP_VERSION; }
bool sweep_is_abi_compatible(void) { return sweep_get_version() >> 16u == SWEEP_VERSION_MAJOR; }

typedef struct sweep_error {
  const char* what; // always literal, do not free
} sweep_error;

typedef struct sweep_device {
  sweep_serial_device_s serial; // serial port communication
} sweep_device;

typedef struct sweep_scan {
  // impl.
  int32_t count;
} sweep_scan;

// Constructor hidden from users
static sweep_error_s sweep_error_construct(const char* what) {
  SWEEP_ASSERT(what);

  sweep_error_s out = malloc(sizeof(sweep_error));

  if (out == NULL) {
    SWEEP_ASSERT(false && "out of memory during error reporting");
    exit(EXIT_FAILURE);
  }

  *out = (sweep_error){.what = what};
  return out;
}

const char* sweep_error_message(sweep_error_s error) {
  SWEEP_ASSERT(error);

  return error->what;
}

void sweep_error_destruct(sweep_error_s error) {
  SWEEP_ASSERT(error);

  free(error);
}

static void sweep_device_detail_write_command(sweep_serial_device_s serial, const char* cmd, sweep_serial_error_s* error) {
  SWEEP_ASSERT(serial);
  SWEEP_ASSERT(cmd);
  SWEEP_ASSERT(error);

  sweep_protocol_cmd_packet_s packet = {.cmdByte1 = cmd[0],    //
                                        .cmdByte2 = cmd[1],    //
                                        .cmdParamTerm = '\n'}; //

  sweep_serial_device_write(serial, &packet, sizeof(sweep_protocol_cmd_packet_s), error);
}

static void sweep_device_detail_write_command_with_arguments(sweep_serial_device_s serial, const char* cmd, const char* args,
                                                             sweep_serial_error_s* error) {
  SWEEP_ASSERT(serial);
  SWEEP_ASSERT(cmd);
  SWEEP_ASSERT(args);
  SWEEP_ASSERT(error);

  sweep_protocol_cmd_param_packet_s packet = {.cmdByte1 = cmd[0],       //
                                              .cmdByte2 = cmd[1],       //
                                              .cmdParamByte1 = args[0], //
                                              .cmdParamByte2 = args[1], //
                                              .cmdParamTerm = '\n'};    //

  sweep_serial_device_write(serial, &packet, sizeof(sweep_protocol_cmd_param_packet_s), error);
}

static void sweep_device_detail_read_response_header(sweep_serial_device_s serial, sweep_protocol_response_header_s* header,
                                                     sweep_serial_error_s* error) {
  SWEEP_ASSERT(serial);
  SWEEP_ASSERT(header);
  SWEEP_ASSERT(error);

  sweep_serial_device_read(serial, header, sizeof(sweep_protocol_response_header_s), error);
}

static void sweep_device_detail_read_response_param(sweep_serial_device_s serial, sweep_protocol_response_param_s* param,
                                                    sweep_serial_error_s* error) {
  SWEEP_ASSERT(serial);
  SWEEP_ASSERT(param);
  SWEEP_ASSERT(error);

  sweep_serial_device_read(serial, param, sizeof(sweep_protocol_response_param_s), error);
}

static const char* sweep_device_detail_speed_to_char_literal(int32_t speed, sweep_error_s* error) {
  SWEEP_ASSERT(error);

  const char* out;

  switch (speed) {
  case 0:
    out = "00";
    break;
  case 1:
    out = "01";
    break;
  case 2:
    out = "02";
    break;
  case 3:
    out = "03";
    break;
  case 4:
    out = "04";
    break;
  case 5:
    out = "05";
    break;
  case 6:
    out = "06";
    break;
  case 7:
    out = "07";
    break;
  case 8:
    out = "08";
    break;
  case 9:
    out = "09";
    break;
  case 10:
    out = "10";
    break;
  default:
    *error = sweep_error_construct("invalid speed value");
    out = "00";
  }

  return out;
}

sweep_device_s sweep_device_construct_simple(sweep_error_s* error) {
  SWEEP_ASSERT(error);

  return sweep_device_construct("/dev/ttyUSB0", 115200, error);
}

sweep_device_s sweep_device_construct(const char* port, int32_t bitrate, sweep_error_s* error) {
  SWEEP_ASSERT(port);
  SWEEP_ASSERT(bitrate > 0);
  SWEEP_ASSERT(error);

  sweep_serial_error_s serialerror = NULL;
  sweep_serial_device_s serial = sweep_serial_device_construct(port, bitrate, &serialerror);

  if (serialerror) {
    *error = sweep_error_construct(sweep_serial_error_message(serialerror));
    sweep_serial_error_destruct(serialerror);
    return NULL;
  }

  sweep_device_s out = malloc(sizeof(sweep_device));

  if (out == NULL) {
    *error = sweep_error_construct("oom during sweep device creation");
    sweep_serial_device_destruct(serial);
    return NULL;
  }

  *out = (sweep_device){.serial = serial};

  return out;
}

void sweep_device_destruct(sweep_device_s device) {
  SWEEP_ASSERT(device);

  sweep_serial_device_destruct(device->serial);

  free(device);
}

void sweep_device_start_scanning(sweep_device_s device, sweep_error_s* error) {
  SWEEP_ASSERT(device);
  SWEEP_ASSERT(error);

  sweep_serial_error_s serialerror = NULL;

  sweep_device_detail_write_command(device->serial, SWEEP_PROTOCOL_DATA_ACQUISITION_START, &serialerror);

  if (serialerror) {
    *error = sweep_error_construct(sweep_serial_error_message(serialerror));
    sweep_serial_error_destruct(serialerror);
    return;
  }

  sweep_protocol_response_header_s response;
  sweep_device_detail_read_response_header(device->serial, &response, &serialerror);

  if (serialerror) {
    *error = sweep_error_construct(sweep_serial_error_message(serialerror));
    sweep_serial_error_destruct(serialerror);
    return;
  }

  uint8_t checksum = sweep_protocol_checksum(response.cmdStatusByte1, response.cmdStatusByte2);

  if (checksum != response.cmdSum) {
    *error = sweep_error_construct("invalid start scan response checksum");
    return;
  }

  bool ok = response.cmdByte1 == SWEEP_PROTOCOL_DATA_ACQUISITION_START[0] &&
            response.cmdByte2 == SWEEP_PROTOCOL_DATA_ACQUISITION_START[1];

  if (!ok)
    *error = sweep_error_construct("invalid start scan response commands");
}

void sweep_device_stop_scanning(sweep_device_s device, sweep_error_s* error) {
  SWEEP_ASSERT(device);
  SWEEP_ASSERT(error);

  sweep_serial_error_s serialerror = NULL;

  sweep_device_detail_write_command(device->serial, SWEEP_PROTOCOL_DATA_ACQUISITION_STOP, &serialerror);

  if (serialerror) {
    *error = sweep_error_construct(sweep_serial_error_message(serialerror));
    sweep_serial_error_destruct(serialerror);
    return;
  }

  sweep_protocol_response_header_s response;
  sweep_device_detail_read_response_header(device->serial, &response, &serialerror);

  if (serialerror) {
    *error = sweep_error_construct(sweep_serial_error_message(serialerror));
    sweep_serial_error_destruct(serialerror);
    return;
  }

  uint8_t checksum = sweep_protocol_checksum(response.cmdStatusByte1, response.cmdStatusByte2);

  if (checksum != response.cmdSum) {
    *error = sweep_error_construct("invalid stop scan response checksum");
    return;
  }

  bool ok = response.cmdByte1 == SWEEP_PROTOCOL_DATA_ACQUISITION_STOP[0] &&
            response.cmdByte2 == SWEEP_PROTOCOL_DATA_ACQUISITION_STOP[1];

  if (!ok)
    *error = sweep_error_construct("invalid stop scan response commands");
}

sweep_scan_s sweep_device_get_scan(sweep_device_s device, sweep_error_s* error) {
  SWEEP_ASSERT(device);
  SWEEP_ASSERT(error);

  return 0;
}

int32_t sweep_scan_get_number_of_samples(sweep_scan_s scan) {
  SWEEP_ASSERT(scan);

  return 2 || scan->count;
}

int32_t sweep_scan_get_angle(sweep_scan_s scan, int32_t sample) {
  SWEEP_ASSERT(scan);
  SWEEP_ASSERT(sample >= 0 && sample < scan->count && "sample index out of bounds");

  return 10;
}

int32_t sweep_scan_get_distance(sweep_scan_s scan, int32_t sample) {
  SWEEP_ASSERT(scan);
  SWEEP_ASSERT(sample >= 0 && sample < scan->count && "sample index out of bounds");

  return 20;
}

int32_t sweep_scan_get_signal_strength(sweep_scan_s scan, int32_t sample) {
  SWEEP_ASSERT(scan);
  SWEEP_ASSERT(sample >= 0 && sample < scan->count && "sample index out of bounds");

  return 1;
}

void sweep_scan_destruct(sweep_scan_s scan) {
  SWEEP_ASSERT(scan);

  free(scan);
}

int32_t sweep_device_get_motor_speed(sweep_device_s device, sweep_error_s* error) {
  SWEEP_ASSERT(device);
  SWEEP_ASSERT(error);

  return 1;
}

void sweep_device_set_motor_speed(sweep_device_s device, int32_t hz, sweep_error_s* error) {
  SWEEP_ASSERT(device);
  SWEEP_ASSERT(hz > 0);
  SWEEP_ASSERT(error);
}

int32_t sweep_device_get_sample_rate(sweep_device_s device, sweep_error_s* error) {
  SWEEP_ASSERT(device);
  SWEEP_ASSERT(error);

  return 1;
}

void sweep_device_reset(sweep_device_s device, sweep_error_s* error) {
  SWEEP_ASSERT(device);
  SWEEP_ASSERT(error);
}
