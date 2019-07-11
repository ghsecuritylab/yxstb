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

    // �ص��Ի���û��ʲô�̳еı�Ҫ��
    void Close();

    // �Ի���ر�ǰ����������������Դ���һЩ�ر��߼���
    // ����false�򲻻����ִ�йرն���������true���������������
    virtual bool onClose();

};

} // namespace Hippo

#endif // __cplusplus

#endif // _Dialog_H_
