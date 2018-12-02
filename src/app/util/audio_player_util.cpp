#include "app/util/audio_player_util.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "app/base/application_context.h"
#include "app/base/resource_loader.h"
#include "app/inf/audio_player.h"

namespace ark {

sp<Future> AudioPlayerUtil::play(const sp<AudioPlayer>& self, const String& src)
{
    const sp<Readable> source = Ark::instance().applicationContext()->resourceLoader()->beanFactory().ensureBuilderByTypeValue<Readable>("ogg", src)->build(nullptr);
    return self->play(source);
}

}
