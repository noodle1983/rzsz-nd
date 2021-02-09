#ifndef VERSION_H
#define VERSION_H

/**
 * The version byte is sent/received in ZF3 in ZRQINIT/ZRINIT/ZFILE.
 * ZF3 is originaly defined as Extended Options:ZTSPARS, which is dropped in this implementation.
 *
 * version 0
 * the same with sz/rz
 *
 * version 1
 * 1. sz/rz dir.
 *      a. support relative path.
 * 2. support large file(>4GB)
 * 3. check existing file's CRC and then resume/skip/retransmit if previous transfer is broke/completed/invalid.
 * 4. options to update client/server working direction
 */
const uint8_t ZVERSION = 1;

#endif /* VERSION_H */
