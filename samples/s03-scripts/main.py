import ark


def main():
    ark.logd("I'm here")
    resource_loader = _application.create_resource_loader('main.xml')
    _application.arena = resource_loader.load(ark.Arena, 'main')

    root_view = resource_loader.refs.root_view

    for i in range(3):
        root_view.add_renderer(_application.arena.load(ark.Renderer, 'label', t='Hello World %d' % i))


if __name__ == '__main__':
    main()
