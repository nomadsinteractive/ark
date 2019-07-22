import ark


def on_clicked(audio_player: ark.AudioPlayer):
    ark.logd('on_red_clicked')
    audio_player.play('s003.ogg')


def main(app: ark.ApplicationFacade):
    resource_loader = app.create_resource_loader('main.xml')
    audio_player = resource_loader.load(ark.AudioPlayer, 'pre:audio_player')
    app.arena = resource_loader.load(ark.Arena, 'main', on_red_clicked=lambda: on_clicked(audio_player))


if __name__ == '__main__':
    main(_application)
