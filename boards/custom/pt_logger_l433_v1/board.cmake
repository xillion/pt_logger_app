board_runner_args(jlink "--device=STM32L433CC" "--speed=4000")
board_runner_args(openocd "--target-openocd-scripts=target/stm32l4x.cfg")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
