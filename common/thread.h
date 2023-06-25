#ifndef __THREAD_H__
#define __THREAD_H__

#include <map>
#include <string>
#include <pthread.h>

class PThread
{
public:
   PThread(const std::string& name = "");
   virtual ~PThread();

   virtual int Start(void *data = 0, pthread_attr_t *attr = 0);
   virtual int End();
   int Wait();
   void Detach();
   bool IsRunning();
   void *GetData(const std::string& label = "_default_");
   void *Lock(bool rw = false, const std::string& label = "_default_");
   void Unlock(const std::string& label = "_default_");
   void *LockPtr(bool rw, void *data);
   void UnlockPtr(void *data);
   static bool LockGlobal(bool rw, void *data);
   static bool UnlockGlobal(void *data);
   static void StartCriticalSection();
   static void EndCriticalSection();
   static bool CancelEnable(bool enable = true);
   static bool CancelDisable();
   static bool CancelAsync(bool enable = true);
   static void CancelTest();
   std::string GetName() { return _name; };
   virtual bool PlugLockable(const std::string& label, void *data);
   void SetAutoDelete(bool value = true);

protected:
   virtual void Run() = 0;
   virtual void OnExit();
   void SetReturnCode(int rc) { _retcode = rc; };
   int GetReturnCode() { return _retcode; };
   std::string _name;

private:
   struct plock_t
   {
      plock_t() : plock(0), ref_count(0) {};
      plock_t(pthread_rwlock_t *p1, int p2 = 1) : plock(p1), ref_count(p2) {};
      pthread_rwlock_t *plock;
      int ref_count;
   };
   struct shared_t
   {
      shared_t() : data(0), locked(false) {};
      shared_t(void *p1, bool p2 = false) : data(p1), locked(p2) {};
      void *data;
      bool locked;
   };
   void SetRunning(bool value);
   void add_plock(void *data);
   void remove_plock(void *data);
   static bool internal_lock(void *data, bool rw);
   static bool internal_unlock(void *data);
   static void *run(void *owner);
   static void exit_handler(void *owner);
   static bool find_plock(void *data, std::map<void *, plock_t>::iterator& it);
   static pthread_rwlock_t _glock;
   static pthread_mutex_t _cs_mutex;
   pthread_rwlock_t _local_lock;
   static std::map<void *, plock_t> _locks;
   std::map<std::string, shared_t> _shared;
   pthread_t _tid;
   bool _running;
   int _retcode;
   bool _auto_delete;
};

#endif
