import ark


def on_red_clicked():
    ark.logd('on_red_clicked')


def main():
    resource_loader = _application.create_resource_loader('main.xml')
    _application.arena = resource_loader.load(ark.Arena, 'main', on_red_clicked=on_red_clicked)


if __name__ == '__main__':
    main()
