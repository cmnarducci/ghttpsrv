#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <csignal>
#include "thread.h"
//#include <iostream>

pthread_rwlock_t PThread::_glock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t PThread::_cs_mutex = PTHREAD_MUTEX_INITIALIZER;
std::map<void *, PThread::plock_t> PThread::_locks;

PThread::PThread(const std::string& name) : _tid(0), _running(false), _retcode(-1), _auto_delete(false)
{
   pthread_rwlock_init(&_local_lock, 0);
   _name = name;
}

PThread::~PThread()
{
   End();
   std::map<std::string, shared_t>::iterator sit;
   for (sit = _shared.begin(); sit != _shared.end(); ++sit)
      remove_plock(sit->second.data);
   pthread_rwlock_destroy(&_local_lock);
   //pthread_rwlock_rdlock(&_glock);
   //std::cout << "### PThread dtor: locks count = " << _locks.size() << std::endl;
   //pthread_rwlock_unlock(&_glock);
}

void *PThread::run(void *owner)
{
   int old_state;
   pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
   PThread *ptr = reinterpret_cast<PThread *>(owner);
   pthread_rwlock_rdlock(&ptr->_local_lock);
   pthread_rwlock_unlock(&ptr->_local_lock);
   pthread_cleanup_push(&PThread::exit_handler, owner);
#ifdef MINGW
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
#endif
   pthread_setcancelstate(old_state, 0);
   ptr->Run();
   pthread_cleanup_pop(1);
#ifdef _WIN64
   pthread_exit((void *)(unsigned long long)ptr->_retcode);
#else
   pthread_exit((void *)(unsigned long)ptr->_retcode);
#endif
   return 0;
}

void PThread::exit_handler(void *owner)
{
   PThread *ptr = reinterpret_cast<PThread *>(owner);
   ptr->OnExit();
   //std::cout << "### exit handler" << std::endl;
   std::map<std::string, shared_t>::iterator sit;
   for (sit = ptr->_shared.begin(); sit != ptr->_shared.end(); ++sit)
   {
      if (sit->second.locked)
      {
         //std::cout << "### found locked" << std::endl;
         internal_unlock(sit->second.data);
         //std::cout << "### unlocked" << std::endl;
      }
   }
   ptr->SetRunning(false);
   if (ptr->_auto_delete)
      delete ptr;
}

int PThread::Start(void *data, pthread_attr_t *attr)
{
   if (IsRunning())
      return -1;
   int old_state;
   pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
   bool changed = (data != _shared["_default_"].data);
   if (_shared["_default_"].data && changed)
      remove_plock(_shared["_default_"].data);
   _shared["_default_"] = data;
   pthread_rwlock_wrlock(&_local_lock);
   int status = pthread_create(&_tid, attr, PThread::run, (void*)this);
   if (status == 0)
   {
      if (_shared["_default_"].data && changed)
         add_plock(_shared["_default_"].data);
      _running = true;
   }
   else
      _shared["_default_"] = 0;
   pthread_rwlock_unlock(&_local_lock);
   pthread_setcancelstate(old_state, 0);
   return status;
}

void PThread::OnExit()
{
}

void PThread::SetAutoDelete(bool value)
{
   _auto_delete = value;
}

void PThread::SetRunning(bool value)
{
   pthread_rwlock_wrlock(&_local_lock);
   _running = value;
   pthread_rwlock_unlock(&_local_lock);
}

bool PThread::IsRunning()
{
   pthread_rwlock_rdlock(&_local_lock);
   bool rc = _running;
   pthread_rwlock_unlock(&_local_lock);
   return rc;
}

bool PThread::PlugLockable(const std::string& label, void *data)
{
   if (!IsRunning() && data)
   {
      std::map<std::string, shared_t>::iterator sit = _shared.find(label);
      if (sit == _shared.end())
      {
         int old_state;
         pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
         _shared[label] = data;
         add_plock(data);
         pthread_setcancelstate(old_state, 0);
         return true;
      }
   }
   return false;
}

void *PThread::GetData(const std::string& label)
{
   std::map<std::string, shared_t>::iterator sit = _shared.find(label);
   if (sit != _shared.end())
      return sit->second.data;
   return 0;
};

bool PThread::internal_lock(void *data, bool rw)
{
   std::map<void *, plock_t>::iterator it;
   if (find_plock(data, it))
   {
      if (rw)
         pthread_rwlock_wrlock(it->second.plock);
      else
         pthread_rwlock_rdlock(it->second.plock);
      return true;
   }
   return false;
}

bool PThread::internal_unlock(void *data)
{
   std::map<void *, plock_t>::iterator it;
   if (find_plock(data, it))
   {
      pthread_rwlock_unlock(it->second.plock);
      return true;
   }
   return false;
}

void *PThread::Lock(bool rw, const std::string& label)
{
   void *rc = 0;
   std::map<std::string, shared_t>::iterator sit = _shared.find(label);
   if (sit != _shared.end() && sit->second.data)
   {
      int old_state;
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
      rc = (internal_lock(sit->second.data, rw)) ? sit->second.data : 0;
      if (rc && pthread_equal(_tid, pthread_self()))
         sit->second.locked = true;
      pthread_setcancelstate(old_state, 0);
   }
   return rc;
}

void PThread::Unlock(const std::string& label)
{
   std::map<std::string, shared_t>::iterator sit = _shared.find(label);
   if (sit != _shared.end() && sit->second.data)
   {
      int old_state;
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
      if (internal_unlock(sit->second.data) && pthread_equal(_tid, pthread_self()))
         sit->second.locked = false;
      pthread_setcancelstate(old_state, 0);
   }
}

void *PThread::LockPtr(bool rw, void *data)
{
   if (data == 0)
      return 0;

   void *rc = 0;
   std::map<std::string, shared_t>::iterator sit;
   for (sit = _shared.begin(); sit != _shared.end(); ++sit)
   {
      if (sit->second.data == data)
      {
         int old_state;
         pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
         rc = (internal_lock(sit->second.data, rw)) ? sit->second.data : 0;
         if (rc && pthread_equal(_tid, pthread_self()))
            sit->second.locked = true;
         pthread_setcancelstate(old_state, 0);
         break;
      }
   }
   return rc;
}

void PThread::UnlockPtr(void *data)
{
   if (data == 0)
      return;

   std::map<std::string, shared_t>::iterator sit;
   for (sit = _shared.begin(); sit != _shared.end(); ++sit)
   {
      if (sit->second.data == data)
      {
         int old_state;
         pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
         if (internal_unlock(sit->second.data) && pthread_equal(_tid, pthread_self()))
            sit->second.locked = false;
         pthread_setcancelstate(old_state, 0);
         break;
      }
   }
}

bool PThread::LockGlobal(bool rw, void *data)
{
   if (data)
      return internal_lock(data, rw);
   return false;
}

bool PThread::UnlockGlobal(void *data)
{
   if (data)
      return internal_unlock(data);
   return false;
}

void PThread::StartCriticalSection()
{
   pthread_mutex_lock(&_cs_mutex);
}

void PThread::EndCriticalSection()
{
   pthread_mutex_unlock(&_cs_mutex);
}

bool PThread::CancelEnable(bool enable)
{
   int oldstate;
   int new_state = (enable) ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE;
   pthread_setcancelstate(new_state, &oldstate);
   return (oldstate == PTHREAD_CANCEL_ENABLE) ? true : false;
}

bool PThread::CancelDisable()
{
   int oldstate;
   pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
   return (oldstate == PTHREAD_CANCEL_ENABLE) ? true : false;
}

bool PThread::CancelAsync(bool enable)
{
   int oldstate;
   int new_state = (enable) ? PTHREAD_CANCEL_ASYNCHRONOUS : PTHREAD_CANCEL_DEFERRED;
   pthread_setcanceltype(new_state, &oldstate);
   return (oldstate == PTHREAD_CANCEL_ASYNCHRONOUS) ? true : false;
}

void PThread::CancelTest()
{
   pthread_testcancel();
}

int PThread::Wait()
{
   int rc = _retcode;
   void *rcv;

   if (_tid != 0)
   {
      pthread_join(_tid, &rcv);
      if (rcv == PTHREAD_CANCELED)
         rc = -1;
      else
#ifdef _WIN64
         rc = (int)(unsigned long long)rcv;
#else
         rc = (int)(unsigned long)rcv;
#endif
      _tid = 0;
      _retcode = 0;
   }
   return rc;
}

void PThread::Detach()
{
   if (_tid != 0)
      pthread_detach(_tid);
}

int PThread::End()
{
   int rc = _retcode;

   if (_tid != 0)
   {
      if (pthread_equal(_tid, pthread_self()) && IsRunning())
#ifdef _WIN64
         pthread_exit((void *)(unsigned long long)_retcode);
#else
         pthread_exit((void *)(unsigned long)_retcode);
#endif
      else
      {
         void *rcv;
         if (IsRunning())
            pthread_cancel(_tid);
         pthread_join(_tid, &rcv);
         if (rcv == PTHREAD_CANCELED)
            rc = -1;
         else
#ifdef _WIN64
            rc = (int)(unsigned long long)rcv;
#else
            rc = (int)(unsigned long)rcv;
#endif
      }
   }
   _tid = 0;
   _retcode = 0;
   return rc;
}

void PThread::add_plock(void *data)
{
   if (data)
   {
      pthread_rwlock_wrlock(&_glock);
      std::map<void *, plock_t>::iterator it = _locks.find(data);
      if (it == _locks.end())
      {
         pthread_rwlock_t *lck = new pthread_rwlock_t;
         pthread_rwlock_init(lck, 0);
         _locks[data] = plock_t(lck, 1);
         //std::cout << "### PThread::add_plock created plock = " << _locks[data].plock;
         //std::cout << ", ref_count = " << _locks[data].ref_count << std::endl;
      }
      else
      {
         it->second.ref_count++;
         //std::cout << "### PThread::add_plock existing plock = " << it->second.plock;
         //std::cout << ", ref_count = " << it->second.ref_count << std::endl;
      }
      pthread_rwlock_unlock(&_glock);
   }
}

void PThread::remove_plock(void *data)
{
   if (data)
   {
      pthread_rwlock_wrlock(&_glock);
      std::map<void *, plock_t>::iterator it = _locks.find(data);
      if (it != _locks.end())
      {
         it->second.ref_count--;
         //std::cout << "### remove_plock: plock = " << it->second.plock;
         //std::cout << ", ref_count = " << it->second.ref_count << std::endl;
         if (it->second.ref_count == 0)
         {
            pthread_rwlock_destroy(it->second.plock);
            delete it->second.plock;
            _locks.erase(it);
            //std::cout << "### remove_plock: lock erased" << std::endl;
         }
      }
      //else
      //   std::cout << "### remove_plock: data not found!!!" << std::endl;
      pthread_rwlock_unlock(&_glock);
   }
}

bool PThread::find_plock(void *data, std::map<void *, plock_t>::iterator& it)
{
   int old_state;
   pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
   pthread_rwlock_rdlock(&_glock);
   it = _locks.find(data);
   bool rc = (it != _locks.end());
   pthread_rwlock_unlock(&_glock);
   pthread_setcancelstate(old_state, 0);
   return rc;
}
