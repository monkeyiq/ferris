
import org.tockit.context.model.BinaryRelation;
import org.tockit.context.model.BinaryRelationImplementation;
import org.tockit.context.model.Context;

import net.sourceforge.toscanaj.controller.fca.ConceptInterpretationContext;
import net.sourceforge.toscanaj.controller.fca.ConceptInterpreter;
import net.sourceforge.toscanaj.controller.fca.DiagramHistory;
import net.sourceforge.toscanaj.controller.fca.DirectConceptInterpreter;
import net.sourceforge.toscanaj.controller.fca.GantersAlgorithm;
import net.sourceforge.toscanaj.controller.fca.LatticeGenerator;
import net.sourceforge.toscanaj.controller.ndimlayout.DefaultDimensionStrategy;
import net.sourceforge.toscanaj.controller.ndimlayout.NDimLayoutOperations;
import net.sourceforge.toscanaj.controller.ndimlayout.MeetIrreducibleChainsDimensionStrategy;
import net.sourceforge.toscanaj.model.diagram.Diagram2D;
import net.sourceforge.toscanaj.model.diagram.DiagramNode;
import net.sourceforge.toscanaj.model.diagram.NestedDiagramNode;
import net.sourceforge.toscanaj.model.diagram.NestedLineDiagram;
import net.sourceforge.toscanaj.model.lattice.Concept;
import net.sourceforge.toscanaj.model.lattice.Lattice;
import net.sourceforge.toscanaj.model.ConceptualSchema;
import net.sourceforge.toscanaj.parser.CSCParser;
import net.sourceforge.toscanaj.parser.CSXParser;
import net.sourceforge.toscanaj.model.manyvaluedcontext.WritableManyValuedContext;
import net.sourceforge.toscanaj.util.xmlize.XMLWriter;

import org.tockit.events.EventBroker;

import java.io.*;


public class FerrisExportConceptLatticeToToscanaJPostprocess extends Object {

    public static void main(String[] args)
        {
            try
            {
                String filename = args[0];
                File schemaFile = new File( filename );
                System.out.println("Loading file:" + filename );
                EventBroker eventBroker = new EventBroker();
                ConceptualSchema conceptualSchema = CSXParser.parse( eventBroker, schemaFile );

                WritableManyValuedContext mvc = conceptualSchema.getManyValuedContext();
                
                Context context = mvc.scale();
                GantersAlgorithm ganter = new GantersAlgorithm();
                Lattice lattice = ganter.createLattice( context );

//                 Diagram2D oldDiagram = conceptualSchema.getDiagram( 0 );
//                 Concept top = oldDiagram.getTopConcept();

                Diagram2D newDiagram =
                    NDimLayoutOperations.createDiagram(
                        lattice,
                        "title",
                        new MeetIrreducibleChainsDimensionStrategy());

                conceptualSchema.addDiagram( newDiagram );

                File outfile = new File( filename + ".new" );
                XMLWriter.write( outfile, conceptualSchema );
            }
            catch( Exception e )
            {
            }
            

//             Lattice lattice2 = lgen.createLattice(context2);
//             Diagram2D outerDiagram = NDimLayoutOperations.createDiagram(lattice1, context1.getName(), DIMENSION_STRATEGY);
            
        }
};

