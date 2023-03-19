//
//  OpenGLComponent.cpp
//  OpenGL 3D App Template - App
//
//  Created by Tim Arterbury on 3/21/20.
//  Copyright © 2020 TesserAct Music Technology LLC. All rights reserved.
//

#include "OpenGLComponent.hpp"


OpenGLComponent::OpenGLComponent()
{
    // Sets the OpenGL version to 3.2
    openGLContext.setOpenGLVersionRequired (OpenGLContext::OpenGLVersion::openGL3_2);

    // Set default 3D orientation for the draggable GUI tool
    draggableOrientation.reset ({ 0.0, 1.0, 0.0 });

    // Attach the OpenGL context
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);
    openGLContext.setContinuousRepainting (true); // Enable rendering

    // Setup OpenGL GUI Overlay Label: Status of Shaders, compiler errors, etc.
    addAndMakeVisible (openGLStatusLabel);
    openGLStatusLabel.setJustificationType (Justification::topLeft);
    openGLStatusLabel.setFont (Font (14.0f));
}

OpenGLComponent::~OpenGLComponent()
{
    openGLContext.setContinuousRepainting (false);
    openGLContext.detach();
}

// OpenGLRenderer Callbacks ================================================
void OpenGLComponent::newOpenGLContextCreated()
{
    compileOpenGLShaderProgram();
    
    vertices = ShapeVertices::generateSquare(); // Setup vertices
    
    // Generate opengl vertex objects ==========================================
    openGLContext.extensions.glGenVertexArrays(1, &VAO); // Vertex Array Object
    openGLContext.extensions.glGenBuffers (1, &VBO);     // Vertex Buffer Object
    
    
    // Bind opengl vertex objects to data ======================================
    openGLContext.extensions.glBindVertexArray (VAO);
//
//    // Fill VBO buffer with vertices array
    openGLContext.extensions.glBindBuffer (juce::gl::GL_ARRAY_BUFFER, VBO);
    openGLContext.extensions.glBufferData (juce::gl::GL_ARRAY_BUFFER,
                                           sizeof (GLfloat) * vertices.size() * 3,
                                           vertices.data(),
                                           juce::gl::GL_STATIC_DRAW);
//
//
//    // Define that our vertices are laid out as groups of 3 GLfloats
    openGLContext.extensions.glVertexAttribPointer (0, 3, juce::gl::GL_FLOAT, juce::gl::GL_FALSE,
                                                    3 * sizeof (GLfloat), NULL);
    openGLContext.extensions.glEnableVertexAttribArray (0);
    
    
    // Optional OpenGL styling commands ========================================
    
    juce::gl::glEnable (juce::gl::GL_BLEND); // Enable alpha blending, allowing for transparency of objects
    juce::gl::glBlendFunc (juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA); // Paired with above line
    
//     juce::gl::glPolygonMode(juce::gl::GL_FRONT_AND_BACK, juce::gl::GL_LINE); // Show wireframe
}

void OpenGLComponent::openGLContextClosing()
{
    // Add any OpenGL related cleanup code here . . .
}

void OpenGLComponent::renderOpenGL()
{
    jassert (OpenGLHelpers::isContextActive());
    
    // Scale viewport
    const float renderingScale = (float) openGLContext.getRenderingScale();
    gl::glViewport (0, 0, roundToInt (renderingScale * getWidth()), roundToInt (renderingScale * getHeight()));

    // Set background color
//    OpenGLHelpers::clear (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

//    // Select shader program
    shaderProgram->use();
//
//    // Setup the Uniforms for use in the Shader
    if (resolution) resolution->set(resolution_juce[0], resolution_juce[1]);
    if (displayScaleFactor) displayScaleFactor->set(displayScaleFactor_juce);
    if (backgroundColor) backgroundColor->set(static_cast<GLfloat>(backgroundColor_juce.getFloatRed()), static_cast<GLfloat>(backgroundColor_juce.getFloatGreen()), static_cast<GLfloat>(backgroundColor_juce.getFloatBlue()));
    if (time){
        const auto time_juce = juce::Time::getMillisecondCounter();
        auto currentTimeSeconds = float(time_juce/1000.0);
        time->set(currentTimeSeconds);
//        std::cout << "Time: " << currentTimeSeconds << std::endl;
    }
    
//
//    // Draw Vertices
    openGLContext.extensions.glBindVertexArray (VAO);
    gl::glDrawArrays (juce::gl::GL_TRIANGLES, 0, (int) vertices.size());
    openGLContext.extensions.glBindVertexArray (0);
}

// JUCE Component Callbacks ====================================================
void OpenGLComponent::paint (Graphics& g)
{
    // You can optionally paint any JUCE graphics over the top of your OpenGL graphics
}

void OpenGLComponent::resized ()
{
    resolution_juce = {getWidth(), getHeight()};
    displayScaleFactor_juce = static_cast<GLfloat>(juce::Desktop::getInstance().getDisplays().displays.getFirst().scale);
    DBG(displayScaleFactor_juce);
    draggableOrientation.setViewport (getLocalBounds());
//    backgroundColor_juce = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    backgroundColor_juce = juce::Colours::black;
    openGLStatusLabel.setBounds (getLocalBounds().reduced (4).removeFromTop (75));
}

void OpenGLComponent::mouseDown (const MouseEvent& e)
{
    draggableOrientation.mouseDown (e.getPosition());
}

void OpenGLComponent::mouseDrag (const MouseEvent& e)
{
    draggableOrientation.mouseDrag (e.getPosition());
}

void OpenGLComponent::handleAsyncUpdate()
{
    openGLStatusLabel.setText (openGLStatusText, dontSendNotification);
}

// OpenGL Related Member Functions =============================================
void OpenGLComponent::compileOpenGLShaderProgram()
{
    std::unique_ptr<OpenGLShaderProgram> shaderProgramAttempt
        = std::make_unique<OpenGLShaderProgram> (openGLContext);

    // Attempt to compile the program
    if (shaderProgramAttempt->addVertexShader ({ BinaryData::BasicVertex_glsl })
        && shaderProgramAttempt->addFragmentShader ({ BinaryData::blob3_glsl })
        && shaderProgramAttempt->link())
    {

        resolution.disconnectFromShaderProgram();
        time.disconnectFromShaderProgram();
        displayScaleFactor.disconnectFromShaderProgram();
        backgroundColor.disconnectFromShaderProgram();
        
        shaderProgram.reset (shaderProgramAttempt.release());

//        std::cout << "UniformLocation: " << juce::gl::glGetUniformLocation(shaderProgram->getProgramID(), "resolution") << std::endl;
        
        resolution.connectToShaderProgram (openGLContext, *shaderProgram);
        time.connectToShaderProgram (openGLContext, *shaderProgram);
        displayScaleFactor.connectToShaderProgram(openGLContext, *shaderProgram);
        backgroundColor.connectToShaderProgram(openGLContext, *shaderProgram);
        
        openGLStatusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
        
    }
    else
    {
        openGLStatusText = shaderProgramAttempt->getLastError();
    }

    triggerAsyncUpdate(); // Update status text
}
