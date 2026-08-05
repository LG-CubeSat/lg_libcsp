#include "../../csp_semaphore.h"

#include <inttypes.h>
#include <csp/csp.h>
#include <csp/csp_debug.h>

#include <time.h>

#if defined(__APPLE__)

/* macOS has no working unnamed POSIX semaphore (sem_init() always fails
 * with ENOSYS), so the binary semaphore is implemented with a mutex and
 * condition variable instead. Semantics mirror the sem_t based version
 * below: post() only wakes a waiter if the semaphore is currently 0. */

void csp_bin_sem_init(csp_bin_sem_t * sem) {

	pthread_mutex_init(&sem->mutex, NULL);
	pthread_cond_init(&sem->cond, NULL);
	sem->value = 1;
}

int csp_bin_sem_wait(csp_bin_sem_t * sem, unsigned int timeout) {

	int ret = CSP_SEMAPHORE_OK;

	pthread_mutex_lock(&sem->mutex);

	if (timeout == CSP_MAX_TIMEOUT) {
		while (sem->value <= 0) {
			pthread_cond_wait(&sem->cond, &sem->mutex);
		}
	} else {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);

		uint32_t sec = timeout / 1000;
		uint32_t nsec = (timeout - 1000 * sec) * 1000000;

		ts.tv_sec += sec;

		if (ts.tv_nsec + nsec >= 1000000000) {
			ts.tv_sec++;
		}

		ts.tv_nsec = (ts.tv_nsec + nsec) % 1000000000;

		while (sem->value <= 0) {
			if (pthread_cond_timedwait(&sem->cond, &sem->mutex, &ts) != 0) {
				ret = CSP_SEMAPHORE_ERROR;
				break;
			}
		}
	}

	if (ret == CSP_SEMAPHORE_OK) {
		sem->value--;
	}

	pthread_mutex_unlock(&sem->mutex);

	return ret;
}

int csp_bin_sem_post(csp_bin_sem_t * sem) {

	pthread_mutex_lock(&sem->mutex);

	if (sem->value <= 0) {
		sem->value = 1;
		pthread_cond_signal(&sem->cond);
	}

	pthread_mutex_unlock(&sem->mutex);

	return CSP_SEMAPHORE_OK;
}

#else

#include <semaphore.h>

void csp_bin_sem_init(csp_bin_sem_t * sem) {

	sem_init((sem_t *) sem, 0, 1);
}

int csp_bin_sem_wait(csp_bin_sem_t * sem, unsigned int timeout) {

	int ret;

	if (timeout == CSP_MAX_TIMEOUT) {
		ret = sem_wait((sem_t *) sem);
	} else {
		struct timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts)) {
			return CSP_SEMAPHORE_ERROR;
		}

		uint32_t sec = timeout / 1000;
		uint32_t nsec = (timeout - 1000 * sec) * 1000000;

		ts.tv_sec += sec;

		if (ts.tv_nsec + nsec >= 1000000000) {
			ts.tv_sec++;
		}

		ts.tv_nsec = (ts.tv_nsec + nsec) % 1000000000;

		ret = sem_timedwait((sem_t *) sem, &ts);
	}

	if (ret != 0)
		return CSP_SEMAPHORE_ERROR;

	return CSP_SEMAPHORE_OK;
}

int csp_bin_sem_post(csp_bin_sem_t * sem) {

	int value;
	sem_getvalue((sem_t *) sem, &value);
	if (value > 0) {
		return CSP_SEMAPHORE_OK;
	}

	if (sem_post((sem_t *) sem) == 0) {
		return CSP_SEMAPHORE_OK;
	}

	return CSP_SEMAPHORE_ERROR;
}

#endif
