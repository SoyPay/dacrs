// Copyright (c) 2012 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_CHECKQUEUE_H_
#define DACRS_CHECKQUEUE_H_

#include <algorithm>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

template<typename T> class CCheckQueueControl;

/** Queue for verifications that have to be performed.
 * The verifications are represented by a type T, which must provide an
 * operator(), returning a bool.
 *
 * One thread (the master) is assumed to push batches of verifications
 * onto the queue, where they are processed by N-1 worker threads. When
 * the master is done adding work, it temporarily joins the worker pool
 * as an N'th worker, until all jobs are done.
 */
template<typename T> class CCheckQueue {
 public:
	// Create a new check queue
	CCheckQueue(unsigned int unBatchSizeIn) :
		m_nIdle(0), m_nTotal(0), m_bAllOk(true), m_unTodo(0), m_bQuit(false), m_unBatchSize(unBatchSizeIn) {
	}

	// Worker thread
	void Thread() {
		Loop();
	}

	// Wait until execution finishes, and return whether all evaluations where succesful.
	bool Wait() {
		return Loop(true);
	}

	// Add a batch of checks to the queue
	void Add(vector<T> &vChecks) {
		boost::unique_lock<boost::mutex> lock(m_Mutex);
		for (auto& check : vChecks) {
			m_vTQueue.push_back(T());
			check.swap(m_vTQueue.back());
		}
		m_unTodo += vChecks.size();
		if (vChecks.size() == 1) {
			m_CondWorker.notify_one();
		} else if (vChecks.size() > 1) {
			m_CondWorker.notify_all();
		}
	}

	~CCheckQueue() {
	}

	friend class CCheckQueueControl<T> ;

 private:
	bool Loop(bool bMaster = false) {
		boost::condition_variable &cond = bMaster ? m_CondMaster : m_CondWorker;
		vector<T> vChecks;
		vChecks.reserve(m_unBatchSize);
		unsigned int unNow = 0;
		bool bOk = true;
		do {
			{
				boost::unique_lock<boost::mutex> lock(m_Mutex);
				// first do the clean-up of the previous loop run (allowing us to do it in the same critsect)
				if (unNow) {
					m_bAllOk &= bOk;
					m_unTodo -= unNow;
					if (m_unTodo == 0 && !bMaster) {
						// We processed the last element; inform the master he can exit and return the result
						m_CondMaster.notify_one();
					}
				} else {
					// first iteration
					m_nTotal++;
				}
				// logically, the do loop starts here
				while (m_vTQueue.empty()) {
					if ((bMaster || m_bQuit) && m_unTodo == 0) {
						m_nTotal--;
						bool bRet = m_bAllOk;
						// reset the status for new work later
						if (bMaster) {
							m_bAllOk = true;
						}
						// return the current status
						return bRet;
					}
					m_nIdle++;
					cond.wait(lock); // wait
					m_nIdle--;
				}
				// Decide how many work units to process now.
				// * Do not try to do everything at once, but aim for increasingly smaller batches so
				//   all workers finish approximately simultaneously.
				// * Try to account for idle jobs which will instantly start helping.
				// * Don't do batches smaller than 1 (duh), or larger than nBatchSize.
				unNow = max(1U, min(m_unBatchSize, (unsigned int) m_vTQueue.size() / (m_nTotal + m_nIdle + 1)));
				vChecks.resize(unNow);
				for (unsigned int i = 0; i < unNow; i++) {
					// We want the lock on the mutex to be as short as possible, so swap jobs from the global
					// queue to the local batch vector instead of copying.
					vChecks[i].swap(m_vTQueue.back());
					m_vTQueue.pop_back();
				}
				// Check whether we need to do work at all
				bOk = m_bAllOk;
			}
			// execute work
			for (auto& check : vChecks)
				if (bOk)
					bOk = check();
			vChecks.clear();
		} while (true);
	}

	// Mutex to protect the inner state
	boost::mutex m_Mutex;
	// Worker threads block on this when out of work
	boost::condition_variable m_CondWorker;
	// Master thread blocks on this when out of work
	boost::condition_variable m_CondMaster;
	// The queue of elements to be processed.
	// As the order of booleans doesn't matter, it is used as a LIFO (stack)
	vector<T> m_vTQueue;

	// The number of workers (including the master) that are idle.
	int m_nIdle;
	// The total number of workers (including the master).
	int m_nTotal;
	// The temporary evaluation result.
	bool m_bAllOk;

	// Number of verifications that haven't completed yet.
	// This includes elements that are not anymore in queue, but still in
	// worker's own batches.
	unsigned int m_unTodo;
	// Whether we're shutting down.
	bool m_bQuit;
	// The maximum number of elements to be processed in one batch
	unsigned int m_unBatchSize;
	// Internal function that does bulk of the verification work.
};

/** RAII-style controller object for a CCheckQueue that guarantees the passed
 *  queue is finished before continuing.
 */
template<typename T> class CCheckQueueControl {
 public:
	CCheckQueueControl(CCheckQueue<T> *pQueueIn) :
		m_pCheckQueue(pQueueIn), m_bDone(false) {
		// passed queue is supposed to be unused, or NULL
		if (m_pCheckQueue != NULL) {
			assert(m_pCheckQueue->nTotal == m_pCheckQueue->nIdle);
			assert(m_pCheckQueue->nTodo == 0);
			assert(m_pCheckQueue->fAllOk == true);
		}
	}

	bool Wait() {
		if (m_pCheckQueue == NULL) {
			return true;
		}
		bool bRet = m_pCheckQueue->Wait();
		m_bDone = true;
		return bRet;
	}

	void Add(vector<T> &vChecks) {
		if (m_pCheckQueue != NULL) {
			m_pCheckQueue->Add(vChecks);
		}
	}

	~CCheckQueueControl() {
		if (!m_bDone) {
			Wait();
		}
	}

 private:
 	CCheckQueue<T> *m_pCheckQueue;
 	bool m_bDone;
};

#endif
