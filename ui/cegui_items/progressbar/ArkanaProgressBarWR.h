#ifndef _ArkanaProgressBarWR_h_
#define _ArkanaProgressBarWR_h_

#include <CEGUIWindowRenderer.h>
#include "ui/cegui_items/progressbar/ArkanaProgressBarWRProperties.h"

#if defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable : 4251)
#endif

// Start of CEGUI namespace section
namespace CEGUI
{
    /*!
    \brief
        ProgressBar class for the FalagardBase module.

        This class requires LookNFeel to be assigned.  The LookNFeel should provide the following:

        States:
            - Enabled
            - Disabled
            - EnabledProgress
            - DisabledProgress

        Named Areas:
            - ProgressArea

        Property initialiser definitions:
            - VerticalProgress - boolean property.
              Determines whether the progress widget is horizontal or vertical.
              Default is horizontal.  Optional.

            - ReversedProgress - boolean property.
              Determines whether the progress grows in the opposite direction to
              what is considered 'usual'.  Set to "True" to have progress grow
              towards the left or bottom of the progress area.  Optional.
    */
    class ArkanaProgressBarWR : public WindowRenderer
    {
    public:
        static const utf8   TypeName[];     //! type name for this widget.

        /*!
        \brief
            Constructor
        */
        ArkanaProgressBarWR(const String& type);

        bool isVertical() const;
        bool isReversed() const;

        void setVertical(bool setting);
        void setReversed(bool setting);

        void render();

    protected:
        // settings to make this class universal.
        bool d_vertical;    //!< True if progress bar operates on the vertical plane.
        bool d_reversed;    //!< True if progress grows in the opposite direction to usual (i.e. to the left / downwards).

        // property objects
        static ArkanaProgressBarWRProperties::VerticalProgress d_verticalProperty;
        static ArkanaProgressBarWRProperties::ReversedProgress d_reversedProperty;
    };

    /*!
    \brief
        Factory for ElasticBox window renderer type
    */
    class ArkanaProgressBarWRFactory : public CEGUI::WindowRendererFactory
    {
    public:
        ArkanaProgressBarWRFactory( void ) : CEGUI::WindowRendererFactory( ArkanaProgressBarWR::TypeName ) { }
        CEGUI::WindowRenderer* create( void );
        void destroy( CEGUI::WindowRenderer* wr );
    };

    ArkanaProgressBarWRFactory& getArkanaProgressBarWRFactory();

} // End of  CEGUI namespace section


#if defined(_MSC_VER)
#	pragma warning(pop)
#endif

#endif  // end of guard _ArkanaProgressBarWR_h_
