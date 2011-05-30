#include "ui/dlg_options_adv_video.h"

#include "3d/3d.h"
#include "ui/ui.h"
#include "ui/cegui_items/simple_list_item.h"
#include "logging/logger.h"
#include "dLib/dConf/configuration.h"

#include <CEGUI.h>

#include <limits> // Infinity.

using namespace FTS;

FTS::AdvVideoDlg::AdvVideoDlg(Configuration* pConf)
    : Dlg("dlg_options_btnVideo_advanced")
    , m_pConf(pConf)
{
    m_bShadersChanged = false;

    if(m_pRoot == NULL)
        return;

    loadData();

    try {
        /* Connect the envents to the member functions. */
        m_pRoot->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                                FTS_SUBS(FTS::AdvVideoDlg::cbCancel));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        m_pRoot->getChild("dlg_options_btnVideo_advanced/btnTestGL")
               ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::AdvVideoDlg::cbTestGL));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    try {
        m_pRoot->getChild("dlg_options_btnVideo_advanced/buttons_frame")
            ->getChild("dlg_options_btnVideo_advanced/btnAccept")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::AdvVideoDlg::cbAccept));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    try {
        m_pRoot->getChild("dlg_options_btnVideo_advanced/buttons_frame")
            ->getChild("dlg_options_btnVideo_advanced/btnCancel")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::AdvVideoDlg::cbCancel));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    try {
        m_pRoot->getChild("dlg_options_btnVideo_advanced/buttons_frame")
            ->getChild("dlg_options_btnVideo_advanced/btnOk")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::AdvVideoDlg::cbOk));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

FTS::AdvVideoDlg::~AdvVideoDlg()
{
}

int FTS::AdvVideoDlg::loadData(void)
{
    CEGUI::Combobox * cb = NULL;
    SimpleListItem *lti = NULL;
    CEGUI::Checkbox *ck = NULL;

    // The multitexturing checkbox
    try {
        ck = (CEGUI::Checkbox *) m_pRoot->getChild("dlg_options_btnVideo_advanced/chkMultiTex");
        ck->setSelected(m_pConf->getBool("MultiTexturing"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    // The complex quads checkbox
    try {
        ck = (CEGUI::Checkbox *) m_pRoot->getChild("dlg_options_btnVideo_advanced/chkComplexQuads");
        ck->setSelected(m_pConf->getBool("ComplexQuads"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    // The anisotropic checkbox
    try {
        ck = (CEGUI::Checkbox *) m_pRoot->getChild("dlg_options_btnVideo_advanced/chkAnisotropic");
        ck->setSelected(m_pConf->getBool("Anisotropic"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    // The model renderer combobox
    try {
        cb = (CEGUI::Combobox *) m_pRoot->getChild("dlg_options_btnVideo_advanced/cbModelRend");

        lti = new SimpleListItem("Shader");
        cb->addItem(lti);
        lti = new SimpleListItem("VBO");
        cb->addItem(lti);
        lti = new SimpleListItem("Vertex Arrays");
        cb->addItem(lti);
        lti = new SimpleListItem("Direct");
        cb->addItem(lti);

        String sCurrent = m_pConf->get("ModelRenderTechnique");
        lti = (SimpleListItem *) cb->findItemWithText(sCurrent, NULL);
        if(!lti) {
            lti = new SimpleListItem(sCurrent);
            cb->addItem(lti);
        }
        lti->setAsDefault(cb);

        cb->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
                           CEGUI::Event::Subscriber(&FTS::AdvVideoDlg::cbShadersChanged,this));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    // The shaders combobox
    try {
        cb = (CEGUI::Combobox *) m_pRoot->getChild("dlg_options_btnVideo_advanced/cbShaders");

        lti = new SimpleListItem("Best");
        cb->addItem(lti);
        lti = new SimpleListItem("Medium");
        cb->addItem(lti);
        lti = new SimpleListItem("Worst");
        cb->addItem(lti);
        lti = new SimpleListItem("None");
        cb->addItem(lti);

        String sCurrent = m_pConf->get("MaxShaderQuality");
        lti = (SimpleListItem *) cb->findItemWithText(sCurrent, NULL);
        if(!lti) {
            lti = new SimpleListItem(sCurrent);
            cb->addItem(lti);
        }
        lti->setAsDefault(cb);

        cb->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
                           FTS_SUBS(FTS::AdvVideoDlg::cbShadersChanged));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }


    return ERR_OK;
}

bool FTS::AdvVideoDlg::cbAccept(const CEGUI::EventArgs & in_ea)
{
    CEGUI::Checkbox *ck = NULL;

    // The multitexturing checkbox
    try {
        ck = (CEGUI::Checkbox *) m_pRoot->getChild("dlg_options_btnVideo_advanced/chkMultiTex");
        m_pConf->set("MultiTexturing", ck->isSelected());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    // The complex quads checkbox
    try {
        ck = (CEGUI::Checkbox *) m_pRoot->getChild("dlg_options_btnVideo_advanced/chkComplexQuads");
        m_pConf->set("ComplexQuads", ck->isSelected());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    // The anisotropic checkbox
    try {
        ck = (CEGUI::Checkbox *) m_pRoot->getChild("dlg_options_btnVideo_advanced/chkAnisotropic");
        m_pConf->set("Anisotropic", ck->isSelected());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    // The shaders quality combobox
    try {
        String sTmp(m_pRoot->getChild("dlg_options_btnVideo_advanced/cbModelRend")->getText());
        if(sTmp != "Shader" &&
           sTmp != "VBO" &&
           sTmp != "Vertex Arrays" &&
           sTmp != "Direct"
          ) {
            sTmp = "Shader";
        }
        m_pConf->set("ModelRenderTechnique", sTmp);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    // The shaders quality combobox
    if(m_bShadersChanged) {
        try {
            /// \TODO GL32 adapt all this stuff!
            String sTmp(m_pRoot->getChild("dlg_options_btnVideo_advanced/cbShaders")->getText());
            m_pConf->set("MaxShaderQuality", sTmp);
        } catch(CEGUI::Exception & e) {
            FTS18N("CEGUI", MsgType::Error, e.getMessage());
        }
    }
    m_pConf->save();
    return true;
}

bool FTS::AdvVideoDlg::cbOk(const CEGUI::EventArgs & in_ea)
{
    this->cbAccept(in_ea);
    this->cbCancel(in_ea);
    return true;
}

bool FTS::AdvVideoDlg::cbShadersChanged(const CEGUI::EventArgs & in_ea)
{
    m_bShadersChanged = true;
    return true;
}

void FTS::AdvVideoDlg::detectMaxVBOSize(int in_mode, double &out_lastOk, double &out_last)
{
    uint32_t uiVBO = 0;
    out_lastOk = 0;
    out_last = 0;
    bool bHitSupremum = false;

    /// \TODO GL32
//     glGenBuffersARB(1, &uiVBO);
//     glBindBufferARB(GL_ARRAY_BUFFER_ARB, uiVBO);
//     for(out_last = 1.0 * 1024.0 * 1024.0 ; out_last * 2.0 < std::numeric_limits<int>::max() - 1.0 ; out_last *= 2.0/*+= 64.0 * 1024.0 * 1024.0*/) {
//         glBufferDataARB((GLenum)GL_ARRAY_BUFFER_ARB, static_cast<GLsizei>(out_last), NULL, in_mode);
//         if(glGetError() == GL_NO_ERROR) {
//             out_lastOk = out_last;
//         } else {
//             bHitSupremum = true;
//             break;
//         }
//     }

    // Whoh shit what a graphics card must that be!! ;-)
    if(!bHitSupremum)
        out_last = std::numeric_limits<double>::infinity();

//     glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0); // Return to client-mode state.
//     glDeleteBuffersARB(1, &uiVBO);
}

bool FTS::AdvVideoDlg::cbTestGL(const CEGUI::EventArgs & in_ea)
{
    int iVal = 0, i2Vals[] = {0, 0};
    String sResults = "";

    /// \TODO: GL32
    /*
    sResults += "Texturing:\n";
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iVal);
    sResults += "    Max texture size: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &iVal);
    sResults += "    Max texture images: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_TEXTURE_COORDS, &iVal);
    sResults += "    Max texture coordinates in a fragment shader: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &iVal);
    sResults += "    Max texture images in a fragment shader: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &iVal);
    sResults += "    Max texture images in a vertex shader: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &iVal);
    sResults += "    Number of compressed texture formats: "+String::nr(iVal)+"\n";

    sResults += "\nVertex Arrays and VBO:\n";
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &iVal);
    sResults += "    Max recommended faces per object: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &iVal);
    sResults += "    Max recommended verts per object: "+String::nr(iVal)+"\n";

    double uiLastOK, uiLast;
    sResults += "\nVBO Approximations:\n";
    this->detectMaxVBOSize(GL_STATIC_DRAW_ARB, uiLastOK, uiLast);
    sResults += "   Static Draw: between "+String::nr(uiLastOK/(1024.0*1024.0), 0)+" and "+String::nr(uiLast/(1024.0*1024.0), 0)+" mB\n";
    this->detectMaxVBOSize(GL_DYNAMIC_DRAW_ARB, uiLastOK, uiLast);
    sResults += "   Dynamic Draw: between "+String::nr(uiLastOK/(1024.0*1024.0), 0)+" and "+String::nr(uiLast/(1024.0*1024.0), 0)+" mB\n";
    this->detectMaxVBOSize(GL_STREAM_DRAW_ARB, uiLastOK, uiLast);
    sResults += "   Stream Draw: between "+String::nr(uiLastOK/(1024.0*1024.0), 0)+" and "+String::nr(uiLast/(1024.0*1024.0), 0)+" mB\n";

    sResults += "\nMisc:\n";
    glGetIntegerv(GL_MAX_LIGHTS, &iVal);
    sResults += "    Max lights: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &iVal);
    sResults += "    Max vertex attributes: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &iVal);
    sResults += "    Max fragment shader uniform parameters: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &iVal);
    sResults += "    Max vertex shader uniform parameters: "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_SAMPLE_BUFFERS, &iVal);
    sResults += "    Max current sample buffers (antialiasing): "+String::nr(iVal)+"\n";
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, i2Vals);
    sResults += "    Max screen size: "+String::nr(i2Vals[0])+"x"+String::nr(i2Vals[1])+"\n";
*/
    sResults += "\nExtensions:\n    ";
    String sExts((const char *)glGetString(GL_EXTENSIONS));
    sExts.replaceStr(" ", "\n    ");
    sResults += sExts + "End";

    FTSMSG(sResults+"\n", MsgType::Raw);
    FTSMSG(sResults, MsgType::Message);

    return true;
}
