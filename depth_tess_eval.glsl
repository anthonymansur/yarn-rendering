#version 410 core
#define pi 3.14159265358979323846f
#define e 2.718281746F
#define EPSILON 0.0001F

// TODO: change hardcoded 64

layout (isolines) in; // patch type: isolines

// input to and output of shader
// ----------------------
patch in vec4 p_1; // left endpoint
patch in vec4 p2; // right endpoint
patch in vec4 tcs_norm[4];
patch in float tcs_dist[4];
patch in int num_of_isolines;
patch in float scaleFactor;

out vec4 _pos;
out vec3 prevPosition;
out vec3 nextPosition;
out float disable;
out float tes_scaleFactor;

// helper functions (see below for definitions)
// ----------------
vec4 computeBezierCurve(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2);

void main()
{
	// init variables 
	// --------------
	vec4 p0 = gl_in[0].gl_Position; // left endpoint of yarn
	vec4 p1 = gl_in[1].gl_Position; // right endpoint of yarn
	float u = gl_TessCoord.x; // location at the curve; from 0 to 1
	/* NOTE: v represents the i-th fiber are are working with. In general, we will have 3 plies, and 
	each ply will have v / 3 fibers. Thus, the i-th fiber corresponds to the mod(v/3)-th ply. 
	*/
	float v = round(num_of_isolines * gl_TessCoord.y); // the i-th fiber we are working with
	// needed for adjacency information for triangle strip in geometry shader 
	float prev = u - 1/64.f;
	float curr = u;
	float next = u + 1/64.f;

	bool prevIsHair = false;
	bool currIsHair = false;
	bool nextIsHair = false;

	if (u < 1/64.f)
	{
		prev = u;
	}
	if (u > 62.5 / 64.f)
	{
		next = u;
	}

	// iterate three times for adjacency information
	for (int i = 0; i < 3; i++)
	{
		if (i == 0)
			u = prev;
		if (i == 1)
			u = curr;
		if (i == 2)
			u = next;

		// Calculate yarn center and its orientation
		// -----------------------------------------
		// Calculate yarn center using input control points
		vec4 yarn_center = computeBezierCurve(u, p_1, p0, p1, p2);

		// fiber center
		vec4 fiber_center = yarn_center;

		//fiber_center = yarn_center; // DEBUG

		if (i == 0) 
			prevPosition = fiber_center.xyz;
		if (i == 1)
		{
			gl_Position = fiber_center;
			disable = 0.f;
		}
		if (i == 2)
		{
			nextPosition = fiber_center.xyz;
			// handle hair cases
			if (!prevIsHair && !currIsHair && nextIsHair)
			{
				// case 1
				gl_Position = vec4(prevPosition, 1);
				disable = 1.f;
			}
			if (!prevIsHair && currIsHair && nextIsHair)
			{
				// case 2 
				gl_Position = vec4(nextPosition, 1);
				disable = 1.f;
			}
			if (prevIsHair && currIsHair && !nextIsHair)
			{
				// case 3
				gl_Position = vec4(prevPosition, 1);
				disable = 1.f;
			}
			if (prevIsHair && !currIsHair && !nextIsHair)
			{
				// case 4
				gl_Position = vec4(prevPosition, 1);
				disable = 1.f;
			}
		}
		_pos = fiber_center; // TODO: is this being used anywhere?
	}

	// pass variables from one shader to next
	tes_scaleFactor = scaleFactor;
}

// Defintion of helper functions
// -----------------------------
vec4 computeBezierCurve(float u, vec4 p_1, vec4 p0, vec4 p1, vec4 p2)
{
	float b0 = (-1.f * u) + (2.f * u * u) + (-1.f * u * u * u);
	float b1 = (2.f) + (-5.f * u * u) + (3.f * u * u * u);
	float b2 = (u) + (4.f * u * u) + (-3.f * u * u * u);
	float b3 = (-1.f * u * u) + (u * u * u);
	return 0.5f * (b0*p_1 + b1*p0 + b2*p1 + b3*p2);
}