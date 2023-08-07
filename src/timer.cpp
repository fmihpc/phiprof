#include <string>
#include <vector>
#include "phiprof.hpp"

using namespace std;

namespace phiprof {
   Timer::Timer(const int id) : id {id} {
      this->start();
   }

   Timer::Timer(const string& label, const vector<string>& groups) : Timer(initializeTimer(label, groups)) {}

   Timer::~Timer() {
      this->stop();
   }

   bool Timer::start() {
      return active ? false : (active = phiprof::start(this->id));
   }

   bool Timer::stop(const double workUnits, const string& workUnitLabel) {
      if (!active)
         return false;

      active = false;
      return phiprof::stop(id, workUnits, workUnitLabel);
   }
}
