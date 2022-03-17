#include "Timestamp.h"
#include <sys/time.h>
#include <stdio.h>
#include <string>
#include <inttypes.h>


using namespace std;

string Timestamp::toString() const {
  	char buf[32] = {0};
  	int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  	int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  	snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  	return buf;
}

Timestamp Timestamp::now() {
  	struct timeval tv;
  	gettimeofday(&tv, NULL);
  	int64_t seconds = tv.tv_sec;
  	return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::nowAfter(double seconds) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds1 = tv.tv_sec;
    int64_t time = seconds1 * kMicroSecondsPerSecond + tv.tv_usec;
    return Timestamp(time + kMicroSecondsPerSecond * seconds);
}

bool operator<(Timestamp l, Timestamp r) {
    return l.microSecondsSinceEpoch() < r.microSecondsSinceEpoch();
}

bool operator==(Timestamp l, Timestamp r) {
    return l.microSecondsSinceEpoch() == r.microSecondsSinceEpoch();
}
