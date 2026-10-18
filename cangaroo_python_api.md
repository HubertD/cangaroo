# CANgaroo Python API Reference

The embedded `cangaroo` module allows you to interact with the CANgaroo measurement backend directly from your scripts.

## `cangaroo.Message` Class

The primary class for constructing and interpreting CAN messages.

### Properties
- `id` (int): The CAN message ID.
- `dlc` (int): The length of the payload in bytes (Data Length Code).
- `extended` (bool): `True` if it is an extended (29-bit) ID, `False` for standard (11-bit).
- `fd` (bool): `True` if the frame is a CAN FD frame.
- `rtr` (bool): `True` if the frame is a Remote Transmission Request.
- `brs` (bool): `True` if the CAN FD frame has Bit Rate Switching enabled.
- `interface_id` (int, read-only): The ID of the interface that received this message.
- `timestamp` (float, read-only): The reception timestamp in seconds.
- `is_rx` (bool, read-only): `True` if the message was received from the bus (RX), `False` if it was transmitted locally (TX).
- `bustype` (str, read-only): The bus type (returns `"CAN"`).

### Methods
- `get_byte(index: int) -> int`: Get the value of a specific data byte (0-63).
- `set_byte(index: int, value: int)`: Set the value of a specific data byte.
- `get_data() -> bytes`: Returns the entire payload as a Python `bytes` object.
- `set_data(data: bytes)`: Sets the payload to the provided bytes and automatically updates the `dlc` property based on the length of the bytes provided (capped at 64).

---

## CAN Interfaces

To send a message to a specific hardware interface, you must provide the `interface_id`. By default, functions use interface `0` (the first available interface).

- **`cangaroo.interfaces() -> list[dict]`**
  Returns a list of dictionaries representing the active interfaces. 
  Example Output: `[{'id': 0, 'name': 'can0'}, {'id': 1, 'name': 'vcan0'}]`

- **`cangaroo.interface_name(id: int) -> str`**
  Returns the human-readable string name of the given interface ID.

---

## Transmission (TX)

- **`cangaroo.send(msg: cangaroo.Message, interface_id: int = 0)`**
  Sends a single CAN message over the specified interface.

- **`cangaroo.send_periodic(msg: cangaroo.Message, interval_ms: int, interface_id: int = 0) -> int`**
  Starts a background periodic transmission of the message at the requested interval. Returns a unique integer `handle` for the periodic task.

- **`cangaroo.stop_periodic(handle: int)`**
  Stops a running periodic background task using the `handle` returned by `send_periodic()`.

---

## Reception (RX)

- **`cangaroo.receive(timeout_sec: float = 1.0) -> list[cangaroo.Message]`**
  Blocks execution until messages are received or the timeout is reached. Returns a list of `cangaroo.Message` objects.

- **`cangaroo.set_filter(id: int, mask: int = 0xFFFFFFFF, extended: bool = None)`**
  Applies a filter to the `receive()` queue. Only messages matching `(received_id & mask) == (id & mask)` will be enqueued.

- **`cangaroo.clear_filter()`**
  Removes the active receive filter, allowing all messages to be caught by `receive()`.

- **`cangaroo.enable_tx_echo(enabled: bool = True)`**
  By default, `receive()` only returns actual incoming (RX) messages. Calling this with `True` causes locally sent messages (TX echo) to also appear in `receive()`.

---

## Trace Window Access

- **`cangaroo.get_trace(count: int = 0) -> list[cangaroo.Message]`**
  Retrieves the last `count` messages directly from the global CANgaroo Trace View. If `count` is 0 or omitted, it retrieves the entire trace buffer.

- **`cangaroo.trace_size() -> int`**
  Returns the total number of messages currently stored in the Trace View.

- **`cangaroo.clear_trace()`**
  Wipes the Trace View history clean.

---

## Measurement Control & Logging

- **`cangaroo.start_measurement() -> bool`**
  Starts the CANgaroo measurement (equivalent to clicking the Play button). Returns `True` on success.

- **`cangaroo.stop_measurement() -> bool`**
  Stops the active CANgaroo measurement.

- **`cangaroo.measurement_running() -> bool`**
  Returns `True` if measurement is currently active.

- **`cangaroo.log_info(text: str)`**
  Logs an informational message to the CANgaroo Log view.

- **`cangaroo.log_warning(text: str)`**
  Logs a warning message (yellow) to the CANgaroo Log view.

- **`cangaroo.log_error(text: str)`**
  Logs an error message (red) to the CANgaroo Log view.
