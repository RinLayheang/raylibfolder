#version 330

// Input fragment texture coordinates (from raylib)
in vec2 fragTexCoord;

// Input fragment color (from raylib)
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Uniforms (values passed from C++ code)
uniform sampler2D textureSampler; // The texture (album art) we are drawing

void main()
{
    // 1. Get the color of the current pixel from the texture
    vec4 texColor = texture(textureSampler, fragTexCoord);
    
    // 2. Normalize coordinates: move origin from (0,0) to center (0.5, 0.5) 
    // and scale to range from -1.0 to 1.0 (so edges are at 1.0)
    vec2 coords = (fragTexCoord - vec2(0.5)) * 2.0; 
    
    // 3. Calculate the distance from the center (0, 0)
    // The length() function calculates sqrt(x*x + y*y)
    float distance = length(coords);
    
    // 4. Check if the pixel is outside the unit circle (distance > 1.0)
    if (distance > 1.0)
    {
        // Discard the pixel by setting the alpha (transparency) to zero
        finalColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
    else
    {
        // Keep the pixel color and apply global color tint/alpha (fragColor)
        finalColor = texColor * fragColor;
    }
}