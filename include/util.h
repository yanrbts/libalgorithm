/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __UTIL_H__
#define __UTIL_H__

/**
 * @param p Pattern string. Can contain normal characters and wildcard characters such as * and ?.
 *        * matches any number of characters (including zero).
 *        ? matches any single character.
 * @param plen The length of the pattern string.
 * @param s The target string is the string to be matched against the pattern.
 * @param slen The length of the target string.
 * @param nocase Whether to ignore case. If nocase is non-zero, character case is ignored when matching.
 * @return Returns 1 if a match is successful, otherwise returns 0
*/
int stringmatchlen(const char *p, int plen, const char *s, int slen, int nocase);

/**
 * Convert a string representing an amount of memory into the number of
 * bytes, so for instance memtoll("1Gb") will return 1073741824 that is
 * (1024*1024*1024).
 * @param p a string representing an amount of memory eg "1Gb"
 * @param err On parsing error, if *err is not NULL, it's set to 1, otherwise it's set to 0
 * @return On error the function return value is 0 otherwise returns bytes number
*/
long long memtoll(const char *p, int *err);

/**
 * @return the UNIX time in microseconds
 */
long long ustime(void);

/**
 * @return the UNIX time in milliseconds
 */
long long mstime(void);

#endif