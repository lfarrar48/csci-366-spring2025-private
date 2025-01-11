import os.path
import re
import shutil
import sys
import json


def iprint(indent: int, v: str = "", prefix: str | None = None):
    if v is None or v.strip() == "":
        return
    indent = "  " * indent
    if prefix:
        indent += prefix
    v = '\n'.join(
        indent + line
        for line in v.splitlines()
    )
    print(v)

def pfx(v: str):
    return "\n".join(" | " + line for line in v.splitlines())

def inform_errors(results, label):
    output = results[label]

    print()
    print(f"=== {label: ^12} ===")

    with open(f"grading/github/{label}.txt", "w") as f:
        if "error" in output:
            print("errored out")
            f.write(pfx(output["error"]))
        else:
            passed = output['passed']
            failed = output['failed']
            total = output['total']
            print("passed/failed/total")
            print(f"{passed: ^6}/{failed: ^6}/{total: ^5}")
            print()

            f.write(f"({label})\n")
            f.write("-" * (2+len(label)))
            f.write("\n\n")
            for test_name, test_results in output['tests'].items():
                if test_results == "passed":
                    print(f" = pass:{test_name}")
                    continue
                print(f" = fail:{test_name}")

                title = re.sub(r"[^\w:_.]+", "", f"{label}::{test_name}")
                f.write(f"--- {title: ^52} ---n")
                f.write(pfx(test_results['stdout']))
                f.write(pfx(test_results['stderr']))
                f.write('\n')
                f.write(f"~~~ {title: ^52} ~~~\n")

    print()


def main():
    if len(sys.argv) != 2:
        print(f"usage: python {sys.argv[0]} [path/to/results.json]", file=sys.stderr)
        exit(1)

    path = sys.argv[1]

    if not os.path.exists(path):
        print('```')
        print("autograder failed! please report this to the instructor!")
        print(" note: there are stdout and stderr files attached to this release for debugging")
        print('```')
        return

    with open(path, "r") as f:
        results = json.load(f)

    print('```')

    summary = results['summary']
    print("SUMMARY")
    for k, v in summary.items():
        print(f"{k}: {v}%")
    print()
    print()

    assigns = set(results.keys()) - {"summary"}
    for assign in assigns:
        inform_errors(results, assign)

    pathy = """\
                           '                        
                       ...,.'..                     
                         ',,'';.                    
                          ...'',                    
                        .''''..'''.                 
                              .'';:c.               
                                ',;:;::'..          
                                 .';;,:c;,,'.       
                                     .,,;,'..'      
                     ..,;.,.         ....'''        
                    .:;x:';'   ...',...''..         
                    .,,::':'   ....;;,:,'           
        .'',          .'''.... ..;:;;'''.           
       ;......    .odl....,''..,'.,c,,,.            
         ....'',.  '',.,....''....;,,;,             
           ..';,';'.....,,,,,;::;,:'..              
          ...'...,''...;,',okOko,'c'..              
        ;,';,...';:c...;:'l0KXX0o';;.               
      ';;lc.......''...'o''lk0Ox:,;;;:              
     ';:c,.,,,,;,'...'.,;;:cllllcc:,cl              
   ..;c;.';;,''.   ..'.':c;,,;;;;;;.;.              
 .'.,;'.''''...    ...'.........'...''              
   .....'...'        .'...,,,;.;;:.;;:              
  ......','',;.       ....,::;.;:;.,;;              
   '.',::::;::cc,       ..''''.'''.''               
    .;;,'',';::..',;::'....'''.''.                  
                  ';;;. ..........';                
               .''.'..,.............':::.           
               .,d...',............ ..'::c          
               .....;::........,.',. ..,llc         
                  ..'cc;.,,.:,.;,... ..'lll         
                  ...:lc....         ..,lll         
                   ..,cc;            ..;cc,         
Bye Friend!"""
    print(pathy)
    print('```')


if __name__ == "__main__":
    main()
