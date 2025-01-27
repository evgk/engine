// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_FLAGS_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_FLAGS_H_

#include "flutter/display_list/display_list_paint.h"
#include "flutter/display_list/types.h"
#include "flutter/fml/logging.h"

namespace flutter {

/// The base class for the classes that maintain a list of
/// attributes that might be important for a number of operations
/// including which rendering attributes need to be set before
/// calling a rendering method (all |drawSomething| calls),
/// or for determining which exceptional conditions may need
/// to be accounted for in bounds calculations.
/// This class contains only protected definitions and helper methods
/// for the public classes |DisplayListAttributeFlags| and
/// |DisplayListSpecialGeometryFlags|.
class DisplayListFlags {
 protected:
  // A drawing operation that is not geometric in nature (but which
  // may still apply a MaskFilter - see |kUsesMaskFilter_| below).
  static constexpr int kIsNonGeometric_ = 0;

  // A geometric operation that is defined as a fill operation
  // regardless of what the current paint Style is set to.
  // This flag will automatically assume |kUsesMaskFilter_|.
  static constexpr int kIsFilledGeometry_ = 1 << 0;

  // A geometric operation that is defined as a stroke operation
  // regardless of what the current paint Style is set to.
  // This flag will automatically assume |kUsesMaskFilter_|.
  static constexpr int kIsStrokedGeometry_ = 1 << 1;

  // A geometric operation that may be a stroke or fill operation
  // depending on the current state of the paint Style attribute.
  // This flag will automatically assume |kUsesMaskFilter_|.
  static constexpr int kIsDrawnGeometry_ = 1 << 2;

  static constexpr int kIsAnyGeometryMask_ =  //
      kIsFilledGeometry_ |                    //
      kIsStrokedGeometry_ |                   //
      kIsDrawnGeometry_;

  // A primitive that floods the surface (or clip) with no
  // natural bounds, such as |drawColor| or |drawPaint|.
  static constexpr int kFloodsSurface_ = 1 << 3;

  static constexpr int kMayHaveCaps_ = 1 << 4;
  static constexpr int kMayHaveJoins_ = 1 << 5;
  static constexpr int kButtCapIsSquare_ = 1 << 6;

  // A geometric operation which has a path that might have
  // end caps that are not rectilinear which means that square
  // end caps might project further than half the stroke width
  // from the geometry bounds.
  // A rectilinear path such as |drawRect| will not have
  // diagonal end caps. |drawLine| might have diagonal end
  // caps depending on the angle of the line, and more likely
  // |drawPath| will often have such end caps.
  static constexpr int kMayHaveDiagonalCaps_ = 1 << 7;

  // A geometric operation which has joined vertices that are
  // not guaranteed to be smooth (angles of incoming and outgoing)
  // segments at some joins may not have the same angle) or
  // rectilinear (squares have right angles at the corners, but
  // those corners will never extend past the bounding box of
  // the geometry pre-transform).
  // |drawRect|, |drawOval| and |drawRRect| all have well
  // behaved joins, but |drawPath| might have joins that cause
  // mitered extensions outside the pre-transformed bounding box.
  static constexpr int kMayHaveAcuteJoins_ = 1 << 8;

  static constexpr int kAnySpecialGeometryMask_ =           //
      kMayHaveCaps_ | kMayHaveJoins_ | kButtCapIsSquare_ |  //
      kMayHaveDiagonalCaps_ | kMayHaveAcuteJoins_;

  // clang-format off
  static constexpr int kUsesAntiAlias_       = 1 << 10;
  static constexpr int kUsesDither_          = 1 << 11;
  static constexpr int kUsesAlpha_           = 1 << 12;
  static constexpr int kUsesColor_           = 1 << 13;
  static constexpr int kUsesBlend_           = 1 << 14;
  static constexpr int kUsesShader_          = 1 << 15;
  static constexpr int kUsesColorFilter_     = 1 << 16;
  static constexpr int kUsesPathEffect_      = 1 << 17;
  static constexpr int kUsesMaskFilter_      = 1 << 18;
  static constexpr int kUsesImageFilter_     = 1 << 19;

  // Some ops have an optional paint argument. If the version
  // stored in the DisplayList ignores the paint, but there
  // is an option to render the same op with a paint then
  // both of the following flags are set to indicate that
  // a default paint object can be constructed when rendering
  // the op to carry information imposed from outside the
  // DisplayList (for example, the opacity override).
  static constexpr int kIgnoresPaint_        = 1 << 30;
  // clang-format on

  static constexpr int kAnyAttributeMask_ =  //
      kUsesAntiAlias_ | kUsesDither_ | kUsesAlpha_ | kUsesColor_ | kUsesBlend_ |
      kUsesShader_ | kUsesColorFilter_ | kUsesPathEffect_ | kUsesMaskFilter_ |
      kUsesImageFilter_;
};

class DisplayListFlagsBase : protected DisplayListFlags {
 protected:
  explicit DisplayListFlagsBase(int flags) : flags_(flags) {}

  const int flags_;

  bool has_any(int qFlags) const { return (flags_ & qFlags) != 0; }
  bool has_all(int qFlags) const { return (flags_ & qFlags) == qFlags; }
  bool has_none(int qFlags) const { return (flags_ & qFlags) == 0; }
};

/// An attribute class for advertising specific properties of
/// a geometric attribute that can affect the computation of
/// the bounds of the primitive.
class DisplayListSpecialGeometryFlags : DisplayListFlagsBase {
 public:
  /// The geometry may have segments that end without closing the path.
  bool may_have_end_caps() const { return has_any(kMayHaveCaps_); }

  /// The geometry may have segments connect non-continuously.
  bool may_have_joins() const { return has_any(kMayHaveJoins_); }

  /// Mainly for drawPoints(PointMode) where Butt caps are rendered as squares.
  bool butt_cap_becomes_square() const { return has_any(kButtCapIsSquare_); }

  /// The geometry may have segments that end on a diagonal
  /// such that their end caps extend further than the default
  /// |strokeWidth * 0.5| margin around the geometry.
  bool may_have_diagonal_caps() const { return has_any(kMayHaveDiagonalCaps_); }

  /// The geometry may have segments that meet at vertices at
  /// an acute angle such that the miter joins will extend
  /// further than the default |strokeWidth * 0.5| margin around
  /// the geometry.
  bool may_have_acute_joins() const { return has_any(kMayHaveAcuteJoins_); }

 private:
  explicit DisplayListSpecialGeometryFlags(int flags)
      : DisplayListFlagsBase(flags) {
    FML_DCHECK((flags & kAnySpecialGeometryMask_) == flags);
  }

  const DisplayListSpecialGeometryFlags with(int extra) const {
    return extra == 0 ? *this : DisplayListSpecialGeometryFlags(flags_ | extra);
  }

  friend class DisplayListAttributeFlags;
};

class DisplayListAttributeFlags : DisplayListFlagsBase {
 public:
  const DisplayListSpecialGeometryFlags WithPathEffect(
      sk_sp<SkPathEffect> effect) const {
    if (is_geometric() && effect) {
      SkPathEffect::DashInfo info;
      if (effect->asADash(&info) == SkPathEffect::kDash_DashType) {
        // A dash effect has a very simple impact. It cannot introduce any
        // miter joins that weren't already present in the original path
        // and it does not grow the bounds of the path, but it can add
        // end caps to areas that might not have had them before so all
        // we need to do is to indicate the potential for diagonal
        // end caps and move on.
        return special_flags_.with(kMayHaveCaps_ | kMayHaveDiagonalCaps_);
      } else {
        // An arbitrary path effect can introduce joins at an arbitrary
        // angle and may change the geometry of the end caps
        return special_flags_.with(kMayHaveCaps_ | kMayHaveDiagonalCaps_ |
                                   kMayHaveJoins_ | kMayHaveAcuteJoins_);
      }
    }
    return special_flags_;
  }

  bool ignores_paint() const { return has_any(kIgnoresPaint_); }

  bool applies_anti_alias() const { return has_any(kUsesAntiAlias_); }
  bool applies_dither() const { return has_any(kUsesDither_); }
  bool applies_color() const { return has_any(kUsesColor_); }
  bool applies_alpha() const { return has_any(kUsesAlpha_); }
  bool applies_alpha_or_color() const {
    return has_any(kUsesAlpha_ | kUsesColor_);
  }

  /// The primitive dynamically determines whether it is a stroke or fill
  /// operation (or both) based on the setting of the |Style| attribute.
  bool applies_style() const { return has_any(kIsDrawnGeometry_); }
  /// The primitive can use any of the stroke attributes, such as
  /// StrokeWidth, StrokeMiter, StrokeCap, or StrokeJoin. This
  /// method will return if the primitive is defined as one that
  /// strokes its geometry (such as |drawLine|) or if it is defined
  /// as one that honors the Style attribute. If the Style attribute
  /// is known then a more accurate answer can be returned from
  /// the |is_stroked| method by supplying the actual setting of
  /// the style.
  // bool applies_stroke_attributes() const { return is_stroked(); }

  bool applies_shader() const { return has_any(kUsesShader_); }
  /// The primitive honors the current SkColorFilter, including
  /// the related attribute InvertColors
  bool applies_color_filter() const { return has_any(kUsesColorFilter_); }
  /// The primitive honors the SkBlendMode or SkBlender
  bool applies_blend() const { return has_any(kUsesBlend_); }
  bool applies_path_effect() const { return has_any(kUsesPathEffect_); }
  /// The primitive honors the SkMaskFilter whether set using the
  /// filter object or using the convenience method |setMaskBlurFilter|
  bool applies_mask_filter() const { return has_any(kUsesMaskFilter_); }
  bool applies_image_filter() const { return has_any(kUsesImageFilter_); }

  bool is_geometric() const { return has_any(kIsAnyGeometryMask_); }
  bool always_stroked() const { return has_any(kIsStrokedGeometry_); }
  bool is_stroked(DlDrawStyle style = DlDrawStyle::kStroke) const {
    return (has_any(kIsStrokedGeometry_) ||
            (style != DlDrawStyle::kFill && has_any(kIsDrawnGeometry_)));
  }

  bool is_flood() const { return has_any(kFloodsSurface_); }

 private:
  explicit DisplayListAttributeFlags(int flags)
      : DisplayListFlagsBase(flags),
        special_flags_(flags & kAnySpecialGeometryMask_) {
    FML_DCHECK((flags & kIsAnyGeometryMask_) == kIsNonGeometric_ ||
               (flags & kIsAnyGeometryMask_) == kIsFilledGeometry_ ||
               (flags & kIsAnyGeometryMask_) == kIsStrokedGeometry_ ||
               (flags & kIsAnyGeometryMask_) == kIsDrawnGeometry_);
    FML_DCHECK(((flags & kAnyAttributeMask_) == 0) !=
               ((flags & kIgnoresPaint_) == 0));
    FML_DCHECK((flags & kIsAnyGeometryMask_) != 0 ||
               (flags & kAnySpecialGeometryMask_) == 0);
  }

  const DisplayListAttributeFlags with(int extra) const {
    return extra == 0 ? *this : DisplayListAttributeFlags(flags_ | extra);
  }

  const DisplayListAttributeFlags without(int remove) const {
    FML_DCHECK(has_all(remove));
    return DisplayListAttributeFlags(flags_ & ~remove);
  }

  const DisplayListSpecialGeometryFlags special_flags_;

  friend class DisplayListOpFlags;
};

class DisplayListOpFlags : DisplayListFlags {
 public:
  static const DisplayListAttributeFlags kSaveLayerFlags;
  static const DisplayListAttributeFlags kSaveLayerWithPaintFlags;
  static const DisplayListAttributeFlags kDrawColorFlags;
  static const DisplayListAttributeFlags kDrawPaintFlags;
  static const DisplayListAttributeFlags kDrawLineFlags;
  // Special case flags for horizonal and vertical lines
  static const DisplayListAttributeFlags kDrawHVLineFlags;
  static const DisplayListAttributeFlags kDrawRectFlags;
  static const DisplayListAttributeFlags kDrawOvalFlags;
  static const DisplayListAttributeFlags kDrawCircleFlags;
  static const DisplayListAttributeFlags kDrawRRectFlags;
  static const DisplayListAttributeFlags kDrawDRRectFlags;
  static const DisplayListAttributeFlags kDrawPathFlags;
  static const DisplayListAttributeFlags kDrawArcNoCenterFlags;
  static const DisplayListAttributeFlags kDrawArcWithCenterFlags;
  static const DisplayListAttributeFlags kDrawPointsAsPointsFlags;
  static const DisplayListAttributeFlags kDrawPointsAsLinesFlags;
  static const DisplayListAttributeFlags kDrawPointsAsPolygonFlags;
  static const DisplayListAttributeFlags kDrawVerticesFlags;
  static const DisplayListAttributeFlags kDrawImageFlags;
  static const DisplayListAttributeFlags kDrawImageWithPaintFlags;
  static const DisplayListAttributeFlags kDrawImageRectFlags;
  static const DisplayListAttributeFlags kDrawImageRectWithPaintFlags;
  static const DisplayListAttributeFlags kDrawImageNineFlags;
  static const DisplayListAttributeFlags kDrawImageNineWithPaintFlags;
  static const DisplayListAttributeFlags kDrawImageLatticeFlags;
  static const DisplayListAttributeFlags kDrawImageLatticeWithPaintFlags;
  static const DisplayListAttributeFlags kDrawAtlasFlags;
  static const DisplayListAttributeFlags kDrawAtlasWithPaintFlags;
  static const DisplayListAttributeFlags kDrawPictureFlags;
  static const DisplayListAttributeFlags kDrawPictureWithPaintFlags;
  static const DisplayListAttributeFlags kDrawDisplayListFlags;
  static const DisplayListAttributeFlags kDrawTextBlobFlags;
  static const DisplayListAttributeFlags kDrawShadowFlags;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_FLAGS_H_
