// Our platform independent renderer class

import Metal
import MetalKit


class Renderer: NSObject, MTKViewDelegate {
    init?(metalKitView: MTKView) {
        super.init()
        ark_runtime_on_create();
        ark_runtime_on_surface_created(Unmanaged.passUnretained(metalKitView).toOpaque())
    }
    
    func draw(in view: MTKView) {
        ark_runtime_on_surface_draw();
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        let bounds = view.bounds
        ark_runtime_on_surface_changed(Int32(size.width), Int32(size.height), Int32(bounds.width), Int32(bounds.height))
    }
}
