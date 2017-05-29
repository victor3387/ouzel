// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#import <Cocoa/Cocoa.h>
#import <IOKit/pwr_mgt/IOPMLib.h>
#include "ApplicationMacOS.h"
#include "CursorResourceMacOS.h"
#include "core/Application.h"
#include "core/Engine.h"
#include "utils/Log.h"

@interface AppDelegate: NSObject<NSApplicationDelegate>

@end

@implementation AppDelegate

-(void)applicationWillFinishLaunching:(__unused NSNotification*)notification
{
    ouzelMain(ouzel::sharedApplication->getArgs());
}

-(void)applicationDidFinishLaunching:(__unused NSNotification*)notification
{
    if (ouzel::sharedEngine)
    {
        ouzel::sharedEngine->start();
    }
}

-(void)applicationWillTerminate:(__unused NSNotification*)notification
{
    ouzel::sharedApplication->exit();

    if (ouzel::sharedEngine)
    {
        ouzel::sharedEngine->stop();
    }
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(__unused NSApplication*)sender
{
    return YES;
}

-(BOOL)application:(__unused NSApplication*)sender openFile:(NSString*)filename
{
    if (ouzel::sharedEngine)
    {
        ouzel::Event event;
        event.type = ouzel::Event::Type::OPEN_FILE;

        event.systemEvent.filename = [filename cStringUsingEncoding:NSUTF8StringEncoding];

        ouzel::sharedEngine->getEventDispatcher()->postEvent(event);
    }

    return YES;
}

-(void)applicationDidBecomeActive:(__unused NSNotification*)notification
{
    ouzel::sharedEngine->resume();
}

-(void)applicationDidResignActive:(__unused NSNotification*)notification
{
    ouzel::sharedEngine->pause();
}

@end

namespace ouzel
{
    ApplicationMacOS::ApplicationMacOS(int aArgc, char* aArgv[]):
        Application(aArgc, aArgv)
    {
        mainQueue = dispatch_get_main_queue();
    }

    int ApplicationMacOS::run()
    {
        @autoreleasepool
        {
            NSApplication* application = [NSApplication sharedApplication];
            [application activateIgnoringOtherApps:YES];

            AppDelegate* appDelegate = [[[AppDelegate alloc] init] autorelease];
            [application setDelegate:appDelegate];
            [application run];
        }

        return EXIT_SUCCESS;
    }

    void ApplicationMacOS::exit()
    {
        Application::exit();

        dispatch_async(mainQueue, ^{
            NSApplication* application = [NSApplication sharedApplication];
            [application terminate:Nil];
        });
    }

    void ApplicationMacOS::execute(const std::function<void(void)>& func)
    {
        std::function<void(void)> localFunction = func;

        dispatch_async(mainQueue, ^{
            localFunction();
        });
    }

    bool ApplicationMacOS::openURL(const std::string& url)
    {
        NSString* nsStringURL = [NSString stringWithUTF8String:url.c_str()];
        NSURL* nsURL = [NSURL URLWithString:nsStringURL];

        return [[NSWorkspace sharedWorkspace] openURL:nsURL] == YES;
    }

    void ApplicationMacOS::setScreenSaverEnabled(bool newScreenSaverEnabled)
    {
        Application::setScreenSaverEnabled(newScreenSaverEnabled);

        dispatch_async(mainQueue, ^{
            if (newScreenSaverEnabled)
            {
                if (noSleepAssertionID)
                {
                    if (IOPMAssertionRelease(noSleepAssertionID) != kIOReturnSuccess)
                    {
                        Log(Log::Level::ERR) << "Failed to enable screen saver";
                    }

                    noSleepAssertionID = 0;
                }
            }
            else
            {
                if (!noSleepAssertionID)
                {
                    CFStringRef reasonForActivity = CFSTR("Ouzel disabling screen saver");
                    
                    if (IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleDisplaySleep,
                                                    kIOPMAssertionLevelOn, reasonForActivity, &noSleepAssertionID) != kIOReturnSuccess)
                    {
                        Log(Log::Level::ERR) << "Failed to disable screen saver";
                    }
                }
            }
        });
    }

    void ApplicationMacOS::activateCursorResource(CursorResource* resource)
    {
        Application::activateCursorResource(resource);

        CursorResourceMacOS* cursorMacOS = static_cast<CursorResourceMacOS*>(resource);

        if (cursorMacOS && cursorMacOS->getNativeCursor())
        {
            [cursorMacOS->getNativeCursor() set];
        }
        else
        {
            [[NSCursor arrowCursor] set];
        }
    }

    CursorResource* ApplicationMacOS::createCursorResource()
    {
        std::lock_guard<std::mutex> lock(resourceMutex);

        std::unique_ptr<CursorResourceMacOS> cursorResource(new CursorResourceMacOS());
        CursorResource* result = cursorResource.get();

        resources.push_back(std::move(cursorResource));

        return result;
    }
}
