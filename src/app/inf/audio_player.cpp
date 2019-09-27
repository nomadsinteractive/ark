#include "app/inf/audio_player.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "app/base/application_context.h"
#include "app/base/resource_loader.h"

namespace ark {

sp<Future> AudioPlayer::play(const sp<AudioPlayer>& self, const String& src, PlayOption options)
{
    BeanFactory& beanFactory = Ark::instance().applicationContext()->resourceLoader()->beanFactory();
    if(self->isAudioFormatSupported(AUDIO_FORMAT_AUTO))
    {
        const sp<Readable> source = beanFactory.ensure<Readable>(src, {});
        return self->play(source, AudioPlayer::AUDIO_FORMAT_AUTO, options);
    }

    String name, ext;
    Strings::rcut(src, name, ext, '.');
    DCHECK(ext, "Unable to guess AudioFormat for \"%s\"", src.c_str());
    const sp<Readable> source = beanFactory.ensure<Readable>(ext, src, {});
    DCHECK(self->isAudioFormatSupported(AUDIO_FORMAT_PCM), "AudioPlayer should support PCM format at least");
    return self->play(source, AUDIO_FORMAT_PCM, options);
}

}
