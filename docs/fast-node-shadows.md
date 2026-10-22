# Optional lightweight node shadows

`NodeStyle` supports `FastShadowEnabled` (default: `false`). Set it in
the `NodeStyle` JSON object alongside `ShadowEnabled` to enable a
cheaper shadow approximation when using QtNodes' `DefaultNodePainter`:

```json
{
  "NodeStyle": {
    "ShadowEnabled": true,
    "FastShadowEnabled": true
  }
}
```

Keep other existing `NodeStyle` JSON fields unchanged. The snippet above
shows only the shadow-related settings.

When enabled, the default painter renders a soft shadow using translucent
rounded rectangles rather than installing a `QGraphicsDropShadowEffect` on
the entire node. Embedded widgets are no longer blurred as part of the
graphics effect. The approximation is not pixel-identical to Qt's blur.

The original rendering is the default. Custom node painters must draw their
own shadows when this option is enabled. This change deliberately does not
modify `QPixmapCache` globally or change embedded-widget cache modes.
Benchmark different graph sizes, viewport zoom levels, and DPI settings
before enabling it application-wide.
