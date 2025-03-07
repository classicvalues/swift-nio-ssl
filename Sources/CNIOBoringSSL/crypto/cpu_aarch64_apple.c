/* Copyright (c) 2021, Google Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <CNIOBoringSSL_cpu.h>

#if defined(OPENSSL_AARCH64) && defined(OPENSSL_APPLE) && \
    !defined(OPENSSL_STATIC_ARMCAP)

#include <sys/sysctl.h>
#include <sys/types.h>

#include <CNIOBoringSSL_arm_arch.h>

#include "internal.h"


extern uint32_t OPENSSL_armcap_P;

static int has_hw_feature(const char *name) {
  int value;
  size_t len = sizeof(value);
  if (sysctlbyname(name, &value, &len, NULL, 0) != 0) {
    return 0;
  }
  if (len != sizeof(int)) {
    // This should not happen. All the values queried should be integer-valued.
    assert(0);
    return 0;
  }

  // Per sys/sysctl.h:
  //
  //   Selectors that return errors are not support on the system. Supported
  //   features will return 1 if they are recommended or 0 if they are supported
  //   but are not expected to help performance. Future versions of these
  //   selectors may return larger values as necessary so it is best to test for
  //   non zero.
  return value != 0;
}

void OPENSSL_cpuid_setup(void) {
  // Apple ARM64 platforms have NEON and cryptography extensions available
  // statically, so we do not need to query them. In particular, there sometimes
  // are no sysctls corresponding to such features. See below.
#if !defined(__ARM_NEON) || !defined(__ARM_FEATURE_CRYPTO)
#error "NEON and crypto extensions should be statically available."
#endif
  OPENSSL_armcap_P =
      ARMV7_NEON | ARMV8_AES | ARMV8_PMULL | ARMV8_SHA1 | ARMV8_SHA256;

  // macOS has sysctls named both like "hw.optional.arm.FEAT_SHA512" and like
  // "hw.optional.armv8_2_sha512". There does not appear to be documentation on
  // which to use. The "armv8_2_sha512" style omits statically-available
  // features, while the "FEAT_SHA512" style includes them. However, the
  // "FEAT_SHA512" style was added in macOS 12, so we use the older style for
  // better compatibility and handle static features above.
  if (has_hw_feature("hw.optional.armv8_2_sha512")) {
    OPENSSL_armcap_P |= ARMV8_SHA512;
  }
}

#endif  // OPENSSL_AARCH64 && OPENSSL_APPLE && !OPENSSL_STATIC_ARMCAP
