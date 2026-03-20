#version 460

layout(set = 0, binding = 2) uniform sampler2D textures[256];

layout(location = 0) in VS_OUT
{
   vec2 fTexCoord;

   flat int fDiffSampl;
   flat int fExtraSampl;
}
fs_in;

void
main(void)
{
   vec4 base = texture(textures[fs_in.fDiffSampl], fs_in.fTexCoord);
   vec4 mask = texture(textures[fs_in.fExtraSampl], fs_in.fTexCoord);

   if ((base * mask).a <= 0.01)
   {
      discard;
   }
}
