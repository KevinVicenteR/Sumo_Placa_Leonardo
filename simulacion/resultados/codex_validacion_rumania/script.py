import sys, pathlib, shutil, subprocess, hashlib, json, os, time, csv
from concurrent.futures import ThreadPoolExecutor
REPO=pathlib.Path.cwd(); TMP=pathlib.Path('/tmp/codex_rumania_validation')
sys.path.insert(0,str(REPO/'simulacion'))
import simular, bateria
bateria.ROUNDS=bateria.ROUNDS_RUMANIA
N=52; START=424242
OUT=REPO/'simulacion/resultados/codex_validacion_rumania'
OUT.mkdir(parents=True,exist_ok=True)
shutil.copy2(__file__,OUT/'script.py')
paths=['estrategia/TablaRedLogica.h','movimiento/TablaRedControl.h']
meta={'semillas':list(range(START,START+N)),'combates_por_casilla':N,'casillas':588,'duracion':30,'versiones':{}}
sources=simular.FUENTES[:]
meta['fuentes_sha256']={str(p.relative_to(REPO)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
for version in ['referencia','entrenada']:
    root=TMP/version; root.mkdir(exist_ok=True)
    shutil.copytree(REPO/'include',root/'include',dirs_exist_ok=True)
    if version=='referencia':
        for p in paths:
            old=subprocess.check_output(['git','show','pesos_neuronales_rumania:include/'+p],cwd=REPO)
            assert old==subprocess.check_output(['git','show',':include/'+p],cwd=REPO)
            (root/'include'/p).write_bytes(old)
    (root/'simulacion').mkdir(exist_ok=True)
    (root/'simulacion/support').symlink_to(REPO/'simulacion/support',target_is_directory=True)
    simular.RAIZ=root; simular.BUILD=root/'build'; simular.BUILD.mkdir(exist_ok=True)
    simular.FUENTES=sources
    binary=simular.compilar({})
    meta['versiones'][version]={'tablas_sha256':{p:hashlib.sha256((root/'include'/p).read_bytes()).hexdigest() for p in paths},'binario_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'compilacion':['c++','-std=c++17','-O2','-w','-I'+str(next(simular.BUILD.glob('include_*'))),'-I'+str(root/'simulacion/support'),*map(str,sources),'-o',str(binary)]}
    cells=[(r,rv,f) for f in bateria.FACTORES for r in bateria.ROUNDS for rv in bateria.RIVALES]
    def run(cell):
        r,rv,f=cell
        extra=bateria.ROBOT+['--dur','30','--semilla',str(START)]+r[1]+rv[2]+f[1]
        rows=simular.ejecutar(binary,rv[1],N,extra)
        assert [int(x['semilla']) for x in rows]==list(range(START,START+N))
        return rows
    results={}; count=0
    with (OUT/(version+'_combates.csv')).open('w') as raw:
        writer=None
        with ThreadPoolExecutor(max_workers=os.cpu_count()) as ex:
            for (r,rv,f),rows in zip(cells,ex.map(run,cells)):
                if writer is None:
                    writer=csv.DictWriter(raw,fieldnames=['round','rival','factor']+list(rows[0])); writer.writeheader()
                for row in rows: writer.writerow(dict(round=r[0],rival=rv[0],factor=f[0],**row))
                results[(r[0],rv[0],f[0])]=(sum(x['gano']=='1' for x in rows),sum(x['cayo']=='1' for x in rows),len(rows),sum(x['cayo']=='1' and x['empujado']!='1' for x in rows))
                count+=1
                if count%84==0: print(version,count,'/588',flush=True)
    assert sum(x[2] for x in results.values())==30576
    bateria.guardar_json(results,OUT/(version+'.json'),meta)
    print(version,'COMPLETA',flush=True)
(OUT/'metadatos.json').write_text(json.dumps(meta,indent=2))
print('FINALIZADO',flush=True)
