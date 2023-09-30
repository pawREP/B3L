#ifdef B3L_HAVE_ASSEMBLERS
    #include "InlineCallbackHook.h"

void B3L::InlineCallback::enable() {
    inlinePatch->enable();
}

void B3L::InlineCallback::disable() {
    inlinePatch->disable();
}

#endif
