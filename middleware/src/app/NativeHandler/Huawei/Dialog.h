#ifndef _Dialog_H_
#define _Dialog_H_

#ifdef __cplusplus

namespace Hippo {

class Message;

class Dialog {
public:
	Dialog();
	virtual ~Dialog();

	virtual bool handleMessage(Message *);

	virtual void draw();

    // 关掉对话框，没有什么继承的必要。
    void Close();

    // 对话框关闭前会调到这里来，可以处理一些关闭逻辑。
    // 返回false则不会继续执行关闭动作。返回true则会正常析构掉。
    virtual bool onClose();

};

} // namespace Hippo

#endif // __cplusplus

#endif // _Dialog_H_
