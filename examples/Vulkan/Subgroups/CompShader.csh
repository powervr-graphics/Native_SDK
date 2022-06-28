#version 320 es
#extension GL_KHR_shader_subgroup_basic : enable
#extension GL_KHR_shader_subgroup_vote : enable
#extension GL_KHR_shader_subgroup_ballot : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable

// Raymarch defines to control the visual effects
#define HIT_RADIUS (0.001)
#define GLOW_RADIUS (0.1)
#define BREAKOUT_COUNT (1000)
#define BREAKOUT_RADIUS (2.0)

// Define the local work group size. These will be passed as specialisation constants to determine best work group size at runtime
layout(local_size_x_id = 0) in;
layout(local_size_y_id = 1) in;

// Pass which subgroup functionality is being used as a series of specialisation constants
layout(constant_id = 2) const bool SUBGROUP_FEATURE_BASIC = false;
layout(constant_id = 3) const bool SUBGROUP_FEATURE_VOTE = false;
layout(constant_id = 4) const bool SUBGROUP_FEATURE_BALLOT = false;
layout(constant_id = 5) const bool SUBGROUP_FEATURE_ARITHMETIC = false;

// Uniform to record the texture to
uniform layout(rgba8, set = 0, binding = 0) writeonly mediump image2D imageOut;

// Uniform to receive camera parameters from
layout(set = 1, binding = 0) uniform CameraStruct
{
	highp mat4 mInvProjMatrix;
	highp mat4 mInvViewMatrix;
}
cameraBuffer;

// Gets the distance from the point to the object. This will be for a sphere at center
float DistanceEstimate(vec3 p)
{
	// Set some initial values for iteration process used to estimate the distance
	vec3 z = p;
	float dr = 1.0;
	float r = 0.0;

	// How do we tell if a point is inside of the mandlebulb?
	// The mandelbulb comes from holomorphic dynamics, we want to test if a point tend to infinity or if it remains trapped
	// after repeated applications of z_n+1 = (z_n)^8 + c where c is the point passed to this function
	for (int i = 0; i < 5; i++)
	{
		// Mandelbulb has a radius of 1.25, so if r is greater than 1.25 the point escapes to infinity
		// and we have an accurate enough of a distance
		r = length(z);
		if (r > 1.25) break;

		// convert z to polar coordinates
		float theta = acos(z.z / r);
		float phi = atan(z.y / z.x);

		// Estimate the derivative at the current point for the distance estimator
		// (dr_n+1)/dz = 8 (dr_n)^7 + 1
		dr = pow(r, 7.0) * 8.0 * dr + 1.0;

		// Raise current point to the power of 8, which in complex plane is multiplying angles by 8
		// and raising the scalar radius to the power of 8
		float zr = pow(r, 8.0);
		theta = theta * 8.0;
		phi = phi * 8.0;

		// convert z back to carteasian coordinates
		z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));

		// Add our starting point back to the value of z_n+1.
		z += p;
	}

	return 0.5 * log(r) * r / dr;
}

// Performs the ray march and returns the color of this ray
vec3 raymarch(const vec3 origin, const vec3 direction, const vec3 hitColor, const vec3 glowColor)
{
	// Local copy of position
	vec3 pos = origin;

	// Set the amount of glow that gets added for every near miss
	vec3 glowContribution = 0.1 * glowColor;
	vec3 glow = vec3(0, 0, 0);
	vec3 color = vec3(0.0);

	// Iterate until we hit or we have a breakout
	for (int i = 0; i < BREAKOUT_COUNT; i++)
	{
		// Calculate distance from the current point to the object
		float distance = DistanceEstimate(pos);

		// If the distance is close enough then consider this a hit and break out
		if (distance <= HIT_RADIUS)
		{
			// How many iterations did it take to
			color = hitColor;
			break;
		}

		// If close enough to cause some glow but not a hit, then accumulate some glow.
		// The contribution of this glow is inversely proportional to the distance to the shape
		if (distance <= GLOW_RADIUS) { glow += ((GLOW_RADIUS - distance) / GLOW_RADIUS) * glowContribution; }

		// If too far away then consider this ray a miss and we can break out
		if (distance >= BREAKOUT_RADIUS) { break; }

		// March the ray forward
		pos += distance * direction;
	}

	// If the user has enabled the arithmetic function then we can do something around the glow or hit
	// For example if there is a hit within this subgroup, then we can ensure that there is atleast some glow
	if (SUBGROUP_FEATURE_ARITHMETIC)
	{
		// Check if this subgroup has had atleast one hit
		float minHit = length(color);
		bool hitHappened = bool(subgroupMin(minHit));

		// Highlight the subgroups that have had at least one hit
		color += float(hitHappened) * vec3(1.0, 0.0, 0.0);
	}

	// Add one last glow contribution to set a background colour
	color += glowContribution;

	// We have either gotten close enough to have found a hit, or far enough away to consider this ray a miss.
	// mix the hit color with the accumulated glow and return
	return color + 0.5 * glow;
}

shared vec2 invocationCount;
void main()
{
	// Set the colour pallet for the demo based on which subgroup features are enabled so the user gets visual feadback that we're using a different pipeline
	vec3 colorPalletHit = vec3(0);
	vec3 colorPalletGlow = vec3(0);

	// The compute only fallback has it's own colour pallet
	if (!SUBGROUP_FEATURE_BASIC)
	{
		colorPalletHit += vec3(123.0, 30.0, 122.0);
		colorPalletGlow += vec3(12.0, 10.0, 62.0);
	}
	else
	{
		colorPalletGlow += vec3(255.0, 30.0, 30.0);
		colorPalletHit += vec3(0, 0, 120);

		// If we're using vote add some more blue
		if (SUBGROUP_FEATURE_VOTE) { colorPalletGlow += vec3(0.0, 0.0, 50); }

		// If we're using ballot add some hit and glow
		if (SUBGROUP_FEATURE_BALLOT) { colorPalletGlow += vec3(0.0, 100, 0.0); }

		// If we're using arithmetic change the hit colour will be changed in ray march
		// to highlight subgroups that have a hit
		// if (SUBGROUP_FEATURE_ARITHMETIC) { colorPalletHit += vec3(0, 0, 255); }
	}

	// Each thread invocation represents one ray being marched. in order to calculate the ray origin, the number of thread invocations has to be calcuated.
	// This is a good opertunity to show off numerous subgroup features for syncronisation, as the calculation is identitcal for all invocations.
	// On the compute fallback every thread invocation is forced to calculate the result. With subgroup basic functionality we can elect just one
	// invocation per subgroup to do the calculation and then synchronise this information across the subgroups
	//
	// In most cases the subgroup can syncronise the shared data across the subgroup by using a subgroup memory barrier. However, if the ballot feature
	// is enabled then the syncronisation can be done using broadcast function instead.
	// ------ Subgroup basic ------ //
	if (SUBGROUP_FEATURE_BASIC)
	{
		// Elect one subgroup to calculate the invocation count
		if (subgroupElect()) { invocationCount = vec2(gl_WorkGroupSize.x * gl_NumWorkGroups.x, gl_WorkGroupSize.y * gl_NumWorkGroups.y); }

		// Syncronisate this value accross all subgroups
		// ------ Subgroup ballot ------ //
		if (SUBGROUP_FEATURE_BALLOT) { invocationCount = subgroupBroadcastFirst(invocationCount); }
		// ------ Subgroup basic ------ //
		else
		{
			subgroupMemoryBarrierShared();
		}
	}
	// ------ Fallback version ------ //
	else
	{
		invocationCount = vec2(gl_WorkGroupSize.x * gl_NumWorkGroups.x, gl_WorkGroupSize.y * gl_NumWorkGroups.y);
	}

	// Each invocation represents one ray cast
	// Get the coordinates of the invocation and translate it into the screen locations
	ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
	vec2 pixelCentre = vec2(pixelCoord) + vec2(0.5);
	vec2 screenSpace = vec2(pixelCentre.x / invocationCount.x, pixelCentre.y / invocationCount.y) * 2.0 - 1.0;

	// Get the ray origin and direction
	vec4 target = cameraBuffer.mInvProjMatrix * vec4(screenSpace.x, screenSpace.y, 1.0, 1.0);
	vec4 origin = cameraBuffer.mInvViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);
	vec4 direction = cameraBuffer.mInvViewMatrix * vec4(normalize(target.xyz), 0);

	// Normalise the generated colour pallet
	colorPalletHit = 0.5f * normalize(colorPalletHit);
	colorPalletGlow = normalize(colorPalletGlow);

	// Performing the Raymarch is very expensive, so we want to skip it as often as possible. We know that the mandlebulb is contained
	// within a sphere of a certain size, we also know that the camera is always a fixed distance away from the camera, so we can draw
	// a circle around the centre of the screen, and if the thread invocation is outside that circle, then we want to skip the raymarch.
	//
	// However, there will be no benifit to skipping on a per thread invocation basis, because both branches inside a subgroup would
	// need to execute by processing each branch one at a time. Instead, the skip is decided on a per subgroup basis.
	//
	// If the subgroup vote feature is enabled, then this allows us to check if every single thread invocation in a subgroup wants to
	// skip. This subgroup will then skip the raymarch if and only if every single thread in the subgroup wants to skip.
	// If subgroup ballot is enabled then a ballot can take place, if a certain percentage of invocations in a subgroup want to skip
	// then the entire subgroup will skip. Allowing for a tighter grouping around the mandlebulb.
	// Finally if neither of these features is enabled then no skipping will take place.
	bool skipRayMarch = false;
	// ------ Subgroup ballot ------ //
	if (SUBGROUP_FEATURE_BALLOT)
	{
		// Is the screenspace coordinates within the radius that the bulb appears in?
		if (length(screenSpace) >= 0.9) { skipRayMarch = true; }

		// Take a ballot on the thread invocations in this subgroup
		uvec4 ballotSkipRaymarch = subgroupBallot(skipRayMarch);

		// Count how many invocations voted to skip, if at least half voted to skip, then make the entire subgroup skip
		if (subgroupBallotBitCount(ballotSkipRaymarch) <= uint(0.5 * float(gl_SubgroupSize))) { skipRayMarch = false; }
		else
		{
			skipRayMarch = true;
		}
	}
	// ------ Subgroup vote ------ //
	else if (SUBGROUP_FEATURE_VOTE)
	{
		// Is the screenspace coordinates within the radius that the bulb appears in?
		if (length(screenSpace) >= 0.9) { skipRayMarch = true; }

		// Only skip the ray march if every invocation in this subgroup wants to skip
		skipRayMarch = subgroupAll(skipRayMarch);
	}

	// Get the colour that will be stored for this pixel by either performing the raymarch
	// or by skipping the raymarch and set this pixel as the background colour
	vec4 color = vec4(0.0);
	if (!skipRayMarch)
	{
		// If this subgroup has decided to perform the raymarch
		color = vec4(raymarch(origin.xyz, direction.xyz, colorPalletHit, colorPalletGlow), 1.0);
	}
	else
	{
		// If this subgroup has decided to skip, then just use the background colour
		color = vec4(0.1 * colorPalletGlow, 1.0);
	}

	// Write out the colour to the offscreen texture
	imageStore(imageOut, pixelCoord, color);
}
