#include<string>


class VectorEngine: public SimObject
{
public:
    class VectorMemPort: public MasterPort
    {
        public:
        VectorMemPort(const std::string& name, VectorEngine& owner,
            uint8_t channels);
        ~VectorMemPort();
        VectorEngine* owner;

        class VecMemTranslation : public BaseMMU::Translation
        {
        protected:
            VectorEngine *owner;

        public:
            Fault fault;
            VecMemTranslation(VectorEngine *_owner) : owner(_owner) {}

            void markDelayed() {}

            void
            finish(const Fault &_fault, const RequestPtr &req,
                gem5::ThreadContext *tc, BaseMMU::Mode mode)
            {
                fault = _fault;
            }
            //with latency?
        };
    };
};