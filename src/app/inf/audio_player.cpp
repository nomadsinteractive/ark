#include "app/inf/audio_player.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/resource_loader.h"

#include "app/base/application_context.h"

namespace ark {

sp<Future> AudioPlayer::play(const sp<AudioPlayer>& self, const String& src, PlayOption options)
{
    BeanFactory& beanFactory = Ark::instance().applicationContext()->resourceLoader()->beanFactory();
    if(self->isAudioFormatSupported(AUDIO_FORMAT_AUTO))
    {
        const sp<Readable> source = beanFactory.ensure<Readable>(src, {});
        return self->play(source, AudioPlayer::AUDIO_FORMAT_AUTO, options);
    }

    auto [name, ext] = src.rcut('.');

    CHECK(ext, "Unable to guess AudioFormat for \"%s\"", src.c_str());
    const sp<Readable> source = beanFactory.ensureBuilderByTypeValue<Readable>(ext, src)->build({});
    CHECK(self->isAudioFormatSupported(AUDIO_FORMAT_PCM), "AudioPlayer should support PCM format at least");
    return self->play(source, AUDIO_FORMAT_PCM, options);
}

}
