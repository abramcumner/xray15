#ifndef _PURE_H_AAA_
#define _PURE_H_AAA_

// messages
#define REG_PRIORITY_LOW		0x11111111ul
#define REG_PRIORITY_NORMAL		0x22222222ul
#define REG_PRIORITY_HIGH		0x33333333ul
#define REG_PRIORITY_CAPTURE	0x7ffffffful
#define REG_PRIORITY_INVALID	0xfffffffful

typedef void __fastcall RP_FUNC		(void *obj);

extern ENGINE_API RP_FUNC rp_Frame;
class ENGINE_API pureFrame
{
public:
	virtual void  OnFrame(void) = 0;
};

extern ENGINE_API RP_FUNC rp_Render;
class ENGINE_API pureRender
{
public:
	virtual void  OnRender(void) = 0;
};

extern ENGINE_API RP_FUNC rp_AppActivate;
class ENGINE_API pureAppActivate
{
public:
	virtual void  OnAppActivate(void) = 0;
};

extern ENGINE_API RP_FUNC rp_AppDeactivate;
class ENGINE_API pureAppDeactivate
{
public:
	virtual void  OnAppDeactivate(void) = 0;
};

extern ENGINE_API RP_FUNC rp_AppStart;
class ENGINE_API pureAppStart
{
public:
	virtual void  OnAppStart(void) = 0;
};

extern ENGINE_API RP_FUNC rp_AppEnd;
class ENGINE_API pureAppEnd
{
public:
	virtual void  OnAppEnd(void) = 0;
};

extern ENGINE_API RP_FUNC rp_DeviceReset;
class ENGINE_API pureDeviceReset
{
public:
	virtual void  OnDeviceReset(void) = 0;
};


//-----------------------------------------------------------------------------
struct _REG_INFO {
	void*	Object;
	int		Prio;
	u32		Flags;
};

ENGINE_API extern int	__cdecl	_REG_Compare(const void *, const void *);

template <class T> class CRegistrator		// the registrator itself
{
	friend ENGINE_API int	__cdecl	_REG_Compare(const void *, const void *);
public:
	xr_vector<_REG_INFO>	R;
	// constructor
	struct {
		u32		in_process	:1;
		u32		changed		:1;
	};
	CRegistrator()			{ in_process=false; changed=false;}

	//
	void Add	(T *obj, int priority=REG_PRIORITY_NORMAL, u32 flags=0)
	{
#ifdef DEBUG
		VERIFY	(priority!=REG_PRIORITY_INVALID);
		VERIFY	(obj);
		for		(u32 i=0; i<R.size(); i++) VERIFY( !((R[i].Prio!=REG_PRIORITY_INVALID)&&(R[i].Object==(void*)obj))   );
#endif
		_REG_INFO			I;
		I.Object			=obj;
		I.Prio				=priority;
		I.Flags				=flags;
		R.push_back			(I);
		
		if(in_process)		changed=true;
		else Resort			( );
	};
	void Remove	(T *obj)
	{
		for (u32 i=0; i<R.size(); i++) {
			if (R[i].Object==obj) R[i].Prio = REG_PRIORITY_INVALID;
		}
		if(in_process)		changed=true;
		else Resort			( );
	};
	void Process(RP_FUNC *f)
	{
		in_process = true;
    	if (R.empty()) return;
		if (R[0].Prio==REG_PRIORITY_CAPTURE)	f(R[0].Object);
		else {
			for (u32 i=0; i<R.size(); i++)
				if(R[i].Prio!=REG_PRIORITY_INVALID)
					f(R[i].Object);

		}
		if(changed)	Resort();
		in_process = false;
	};
	void Resort	(void)
	{
		qsort	(&*R.begin(),R.size(),sizeof(_REG_INFO),_REG_Compare);
		while	((R.size()) && (R[R.size()-1].Prio==REG_PRIORITY_INVALID)) R.pop_back();
		if (R.empty())		R.clear		();
		changed				= false;
	};
};

#endif
