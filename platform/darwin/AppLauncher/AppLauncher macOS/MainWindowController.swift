//
//  MainWindowController.swift
//  AppLauncher macOS
//


import Cocoa

class MainWindowController: NSWindowController {

    override func windowDidLoad() {
        super.windowDidLoad()
        if let window = window {
            var width: Float = 0
            var height: Float = 0
            ark_runtime_get_render_resolution(&width, &height);
            window.setContentSize(CGSize(width: CGFloat(width), height: CGFloat(height)))
        }
    }
    
    override func mouseDown(with event: NSEvent) {
        self.onMouseEvent(0, with: event)
    }
    
    override func mouseUp(with event: NSEvent) {
        self.onMouseEvent(1, with: event)
    }
    
    override func mouseMoved(with event: NSEvent) {
        self.onMouseEvent(2, with: event)
    }
    
    func onMouseEvent(_ type: UInt32, with event: NSEvent) {
        let pos = event.locationInWindow
        ark_runtime_on_touch(type, Float(pos.x), Float(pos.y), UInt32(event.timestamp), 1)
    }
}
