#pragma once

#include "xdg-shell-protocol.h"

namespace wall {

class XdgBase {
   public:
    XdgBase(xdg_wm_base* base);
    virtual ~XdgBase();

    XdgBase(XdgBase&&) = delete;
    XdgBase(const XdgBase&) = delete;
    auto operator=(const XdgBase&) -> XdgBase = delete;
    auto operator=(XdgBase&&) -> XdgBase = delete;

    auto set_wm_base(xdg_wm_base* base) -> void;

   protected:
   private:
    static const struct xdg_wm_base_listener k_listener;

    xdg_wm_base* m_base{};
};
}  // namespace wall
