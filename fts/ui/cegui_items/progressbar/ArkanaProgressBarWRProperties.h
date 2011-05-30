#ifndef _ArkanaProgressBarWRProperties_h_
#define _ArkanaProgressBarWRProperties_h_

#include <CEGUIProperty.h>

// Start of CEGUI namespace section
namespace CEGUI
{

/*!
\brief
	Namespace containing the specialised properties interface for the Progress Bar under Falagard class
*/
namespace ArkanaProgressBarWRProperties
{
    /*!
    \brief
        Property to access the setting that controls whether the progress bar is horizontal or vertical.

        \par Usage:
            - Name: VerticalProgress
            - Format: "[text]".

        \par Where [Text] is:
            - "True" to indicate the progress bar's operates in the vertical direction.
            - "False" to indicate the progress bar's operates in the horizontal direction.
    */
    class VerticalProgress : public Property
    {
    public:
        VerticalProgress() : Property(
            "VerticalProgress",
            "Property to get/set whether the ProgressBar operates in the vertical direction.  Value is either \"True\" or \"False\".",
            "False")
        {}

        String	get(const PropertyReceiver* receiver) const;
        void	set(PropertyReceiver* receiver, const String& value);
    };

    /*!
    \brief
        Property to access the setting that controls the direction that progress 'grows' towards

        \par Usage:
            - Name: ReversedProgress
            - Format: "[text]".

        \par Where [Text] is:
            - "True" to indicate the progress grows towards the left or bottom edge.
            - "False" to indicate the progress grows towards the right or top edge.
    */
    class ReversedProgress : public Property
    {
    public:
        ReversedProgress() : Property(
            "ReversedProgress",
            "Property to get/set whether the ProgressBar operates in reversed direction.  Value is either \"True\" or \"False\".",
            "False")
        {}

        String	get(const PropertyReceiver* receiver) const;
        void	set(PropertyReceiver* receiver, const String& value);
    };

}

} // End of  CEGUI namespace section


#endif  // end of guard _ArkanaProgressBarWR_h_
