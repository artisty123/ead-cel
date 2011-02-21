#include <Timer.h>
#include <gtest/gtest.h>
#include <iostream>
#include <errno.h>
using namespace std;

namespace {

    class TimerTest : public ::testing::Test
    {
    protected:
        int _timeouts, _interval;
        Milliseconds _lastTimeout;

        TimerTest() {};
        //virtual ~TimerTest() {};
        //virtual void SetUp() {};
        //virtual void TearDown() {};

    public:
        void timerCallback();

    };

    class TimerBaseTest : public ::testing::Test
    {
    protected:
        TimerBaseTest() {};
        //virtual ~TimerTest() {};
        //virtual void SetUp() {};
        //virtual void TearDown() {};

    public:
        void * timerCallback(void * param);

    };

    class MyTimerBase : public TimerBase
    {
    private:
        int _timeouts;
        Milliseconds _lastTimeout;

    public:
        MyTimerBase(bool singleShot = false) :
            TimerBase(singleShot),
            _timeouts(0), _lastTimeout(0)
        {
        };

        int getTimeouts()
        {
            return _timeouts;
        };

        virtual int start(int msec)
        {
            _timeouts = 0;
            _lastTimeout.setTimestamp();
            return TimerBase::start(msec);
        }

    protected:
        virtual void timeout()
        {
            // calcula tempo que passou desde a �ltima chamada ao timeout
            Milliseconds diff;
            diff -= _lastTimeout;
            _lastTimeout.setTimestamp();

            // d� um limite m�ximo de 2ms de erro para cima ou para baixo
            cout << "Timeout " << _timeouts << ", diff " << diff.getTime() << endl;
            EXPECT_TRUE((diff.getTime() <= getInterval().getTime() + 2) &&
                        (diff.getTime() >= getInterval().getTime() - 2))
                        << "Diferen�a: " << diff.getTime()
                        << ", Intervalo " << getInterval().getTime();

            _timeouts++;
        };

    };

};
