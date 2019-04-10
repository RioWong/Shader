varying vec2 g_pPosition; //��ǰ������Ļ�е�����

void main()
{
	vec4 pjPosition = vec4(gl_ModelViewProjectionMatrix * gl_Vertex); //����ǰ��ת������Ļ����ϵ��
	g_pPosition = vec2(pjPosition.x/pjPosition.w, pjPosition.y/pjPosition.w); 

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}
