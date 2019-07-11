#ifndef IPConflictSetting_h
#define IPConflictSetting_h

#ifdef __cplusplus

class IPConflictSetting {
public:
    IPConflictSetting();
    ~IPConflictSetting();

    IPConflictSetting& operator = (const IPConflictSetting& rhs);

    void setReplyTime(int replyTime);
    const int getReplyTime() const;

    void setConflictTime(int time);
    const int getConflictTime() const;

    void setUnconflictTime(int time);
    const int getUnconflictTime() const;

private:
    int mReplyTime; //ARPӦ��ʱ��
    int mConflictTime; //��ͻʱ�������ʱ��
    int mUnconflictTime; //����ͻʱ�������ʱ��
};

#endif

#endif
