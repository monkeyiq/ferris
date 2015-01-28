//
// These two are from the examples directory in dbxml 1.1
//
#include "dbxml/DbXml.hpp"
#include "db_cxx.h"
#include <Ferris/FerrisException.hh>

namespace Ferris
{
    
class myDbEnv
{
	public:
		
		myDbEnv( const std::string &envHome ) 
			: environmentHome_( envHome ),
			dbEnv_( 0 ),
			dbEnvIsOpen_( false ),
    		cFlags_(DB_CREATE|DB_INIT_LOCK|DB_INIT_LOG|DB_INIT_MPOOL|DB_INIT_TXN)
		{
			if ( environmentHome_.empty() )
			{
                fh_stringstream ss;
                ss << "Error opening the database environment. Found empty path string. " << std::endl;
                Throw_dbXMLException( tostr(ss), 0 );
			}
            else
            {
				try
				{
					//redirect debugging information to std::cerr
//					dbEnv_.set_error_stream(&std::cerr);

					//open the environment
        			dbEnv_.open(environmentHome_.c_str(),cFlags_, 0);
					dbEnvIsOpen_ = true;
    			}
				//if an exception gets caught here, almost certainly there will be 
				//  problems down the road. So simply exit rather than risk
				//  further problems.
    			catch(DbException &e)
    			{
                    fh_stringstream ss;
        			ss << "Error opening database environment: " << environmentHome_ << std::endl;
        			ss << e.what() << std::endl;
                    Throw_dbXMLException( tostr(ss), 0 );
    			}
    			catch(std::exception &e)
    			{
                    fh_stringstream ss;
        			ss << "Error opening database environment: " << environmentHome_ << std::endl;
        			ss << e.what() << std::endl;
                    Throw_dbXMLException( tostr(ss), 0 );
    			}
			}

		} //end myDbEnv constructor

		~myDbEnv()
		{
			if ( dbEnvIsOpen_ )
			{
				//close the db env.
				try
				{
					dbEnv_.close(0);
				}		 
    			catch(DbException &e)
    			{
                    fh_stringstream ss;
        			ss << "Error closing database environment: " << environmentHome_ << std::endl;
        			ss << e.what() << std::endl;
                    Throw_dbXMLException( tostr(ss), 0 );
    			}
    			catch(std::exception &e)
    			{
                    fh_stringstream ss;
        			ss << "Error closing database environment: " << environmentHome_ << std::endl;
        			ss << e.what() << std::endl;
                    Throw_dbXMLException( tostr(ss), 0 );
    			}

			}

		} 

		inline const std::string &getDbEnvHome() const { return environmentHome_; }
		inline DbEnv &getDbEnv() { return dbEnv_; }		

															  

	private:
		DbEnv dbEnv_;
		std::string environmentHome_;
    	u_int32_t cFlags_;
		bool dbEnvIsOpen_;


};

#include <dbxml/DbXml.hpp>
#include <dbxml/XmlManager.hpp>
#include "db_cxx.h"

//Forward declare
class myDbEnv;

class myXmlContainer
{
	public:


    //Constructor that opens a container without a db environment.
    myXmlContainer( DbXml::XmlManager* manager, const std::string &containerName ) 
        : containerName_( Ferris::CleanupURL( containerName ) ), 
          cFlags_(DB_CREATE), 
          container_( manager->createContainer( containerName ) )
		{
		}

    void sync()
        {
            container_.sync();
        }
    
    void updateDocument( DbXml::XmlTransaction& txnid,
                         DbXml::XmlDocument &document,
                         DbXml::XmlUpdateContext& context )
        {
            getContainer().updateDocument( txnid, document, context );
        }
    void updateDocument( DbXml::XmlDocument &document,
                         DbXml::XmlUpdateContext& context )
        {
            getContainer().updateDocument( document, context );
        }


    
    

    //get the container managed by this class
    inline DbXml::XmlContainer &getContainer() { return container_; }

    //get this container's name
    inline const std::string &getContainerName() { return containerName_; }

    ~myXmlContainer()
		{
				try
				{
                    container_.sync();
				} 
				//XmlException is derived from std::exception, so this catches XmlException
    			catch(std::exception &e)
    			{
                    fh_stringstream ss;
        			ss << "Error closing container: " << containerName_ << std::endl;
        			ss << e.what() << std::endl;
                    Throw_dbXMLException( tostr(ss), 0 );
    			}
		}

private:

    DbXml::XmlContainer container_;
    std::string containerName_;
    u_int32_t cFlags_;


};

 
};
