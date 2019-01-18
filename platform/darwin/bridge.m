#import <AppKit/NSWindow.h>

NSView* getContentView(NSWindow* window)
{
    return window.contentView;
}
