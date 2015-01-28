
namespace Ferris
{

    template <class P>
    struct FerrisSmartPointerChecker
    {
        FerrisSmartPointerChecker() {}
        template <class P1> FerrisSmartPointerChecker(const FerrisSmartPointerChecker<P1>&) {}
        template <class P1> FerrisSmartPointerChecker(const Loki::NoCheck<P1>&) {}
        template <class P1> FerrisSmartPointerChecker(const Loki::AssertCheck<P1>&) {}
        
        static void OnDefault(const P&) {}
        static void OnInit(const P&) {}
        static void OnDereference(P val)
            {
                if( !val ) { ::Ferris::BackTrace(); assert(val); }
            }
        static void Swap(FerrisSmartPointerChecker&) {}
    };
 
};
