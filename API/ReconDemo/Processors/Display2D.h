#ifndef DISPLAY_H
#define DISPLAY_H

#include <vector>
#include <future>
#include <thread>
#include <QDebug>
#include <utility>
#include <functional>
#include <chrono>
#include "Implement/ProcessorImpl.h"

class ImageLayoutWidget;

class Display2D :
        public Yap::ProcessorImpl
{
    IMPLEMENT_SHARED(Display2D)
public:
    explicit Display2D(ImageLayoutWidget& display_window);
    Display2D(const Display2D& rhs);

    virtual bool Input(const wchar_t * port, Yap::IData * data) override;


protected:
    ~Display2D();
    ImageLayoutWidget& _display_window;

};

#endif // DISPLAY_H
