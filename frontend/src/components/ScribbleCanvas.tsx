import { useCallback, useRef, useState } from 'react';
import { CanvasPath, ReactSketchCanvas, ReactSketchCanvasRef } from 'react-sketch-canvas';

type StrokeColors =
	| 'black'
	| 'red'
	| 'blue'
	| 'green'
	| 'yellow'
	| 'orange'
	| 'purple'
	| 'pink'
	| 'brown';

export function ScribbleCanvas() {
	const sketchRef = useRef<ReactSketchCanvasRef>(null);
    const [path, setPath] = useState<CanvasPath[]>([]);
	const [strokeColor, setStrokeColor] = useState<StrokeColors>('black');

	const onChange = useCallback((paths: CanvasPath[]) => {
		// Get the difference between the previous and current paths
        const newPaths = paths.filter((path) => !path.);
        setPath(newPaths);
	}, []);

	return (
		<div className="relative w-[800px] h-[600px]">
			<ReactSketchCanvas
				ref={sketchRef}
				style={{ border: '1px solid black' }}
				onChange={onChange}
				strokeWidth={5}
				strokeColor={strokeColor}
				canvasColor="white"
				width="100%"
				height="100%"
			/>
			<div className="absolute top-0 right-0 m-2 z-10">
				<button onClick={() => setStrokeColor('black')}>Black</button>
				<button onClick={() => setStrokeColor('red')}>Red</button>
				<button onClick={() => setStrokeColor('blue')}>Blue</button>
				<button onClick={() => setStrokeColor('green')}>Green</button>
				<button onClick={() => setStrokeColor('yellow')}>Yellow</button>
				<button onClick={() => setStrokeColor('orange')}>Orange</button>
			</div>
		</div>
	);
}
